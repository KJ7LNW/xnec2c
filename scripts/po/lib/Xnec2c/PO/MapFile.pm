package Xnec2c::PO::MapFile;

use strict;
use warnings;
use utf8;

use Exporter qw(import);
use File::Basename qw(basename dirname);
use File::Temp;

our @EXPORT_OK = qw(
	OWNER_MODEL OWNER_PROGRAM decode_map_value encode_map_value
	read_manifest_records read_map_records read_utf8_file record_line
	record_owner_tags record_tag_order serialize_records source_excerpt
	write_utf8_file
);

# Enumerate the parties writing one record.
use constant OWNER_PROGRAM => 'program';
use constant OWNER_MODEL => 'model';

# One record reads as one tag per line: an uppercase tag, a tab, then its
# value. K names the record, C and S name the catalog entry it answers, L names
# the language, T holds the translation, and X holds the exemption a source
# with no distinct target form carries. Each tag also names the party writing
# it.
my @RECORD_FORMAT = (
	{ tag => 'K', optional => 0, owner => OWNER_PROGRAM },
	{ tag => 'C', optional => 1, owner => OWNER_PROGRAM },
	{ tag => 'S', optional => 0, owner => OWNER_PROGRAM },
	{ tag => 'L', optional => 0, owner => OWNER_PROGRAM },
	{ tag => 'T', optional => 0, owner => OWNER_MODEL },
	{ tag => 'X', optional => 1, owner => OWNER_MODEL },
);
my %RECORD_TAG_MAP = map { $_->{tag} => $_ } @RECORD_FORMAT;

# Every escape a map value carries, keyed by the character the backslash names.
my %ESCAPE_MAP = (
	n => "\n",
	t => "\t",
	r => "\r",
	s => ' ',
	'"' => '"',
	'\\' => '\\',
);

# Read a complete UTF-8 text file.
sub read_utf8_file
{
	my ($path) = @_;

	open my $file, '<:encoding(UTF-8)', $path
		or die "$path: unable to read: $!\n";
	my $content = do { local $/; <$file> };
	close $file or die "$path: unable to close after reading: $!\n";

	return $content;
}

# Write a complete UTF-8 text file, replacing any existing one through a single
# rename. An output map holds answers no other file carries, so an interrupted
# rewrite must leave the previous content whole.
sub write_utf8_file
{
	my ($path, $content) = @_;

	# Keep automatic cleanup through the write itself; the rename below takes
	# ownership only once the complete content reached the filesystem.
	my $file = File::Temp->new(TEMPLATE => '.' . basename($path) . '.XXXXXX',
		DIR => dirname($path), UNLINK => 1);
	my $staged = $file->filename;

	binmode $file, ':encoding(UTF-8)'
		or die "$staged: unable to set UTF-8 encoding: $!\n";
	print {$file} $content
		or die "$staged: unable to write content: $!\n";
	close $file or die "$staged: unable to close after writing: $!\n";
	rename $staged, $path or die "$path: unable to replace: $!\n";
	$file->unlink_on_destroy(0);
}

# Encode one map value without raw record delimiters.
sub encode_map_value
{
	my ($value) = @_;

	$value =~ s/\\/\\\\/g;
	$value =~ s/\n/\\n/g;
	$value =~ s/\t/\\t/g;
	$value =~ s/\r/\\r/g;
	$value =~ s/"/\\"/g;

	# Spaces at either edge survive no round trip through a writer that trims
	# them, so each one leaves as a visible escape and returns as a space.
	$value =~ s/\A(\x20+)/'\\s' x length($1)/e;
	$value =~ s/(\x20+)\z/'\\s' x length($1)/e;

	return $value;
}

# Decode one map value and reject unsupported escape sequences.
sub decode_map_value
{
	my ($path, $record_key, $line, $value) = @_;
	my @faults;

	$value =~ s/\\(.?)/
		exists $ESCAPE_MAP{$1}
			? $ESCAPE_MAP{$1}
			: do {
				push @faults, "$path:$line: K $record_key has "
					. ($1 eq '' ? 'a trailing lone backslash; write \\\\'
						: "unsupported escape \\$1");
				'';
			}
	/gex;

	return { value => $value, faults => \@faults };
}

# Quote one source string for a message, escaped through the map codec and
# clipped, so every fault reads on a single line.
sub source_excerpt
{
	my ($source) = @_;
	return encode_map_value(substr($source, 0, 60));
}

# Read one parsed tag's source line, using the required key line when an
# optional tag is absent.
sub record_line
{
	my ($record, $tag) = @_;

	return $record->{lines}{$tag} // $record->{lines}{K};
}

# Name the record tags in the sole authoritative order, the order every
# message and prompt cites rather than restating.
sub record_tag_order
{
	return join(' ', map { $_->{tag} } @RECORD_FORMAT);
}

# Name the tags written by one record owner.
sub record_owner_tags
{
	my ($owner) = @_;

	return map { $_->{tag} }
		grep { $_->{owner} eq $owner } @RECORD_FORMAT;
}

# Serialize records in the sole authoritative tag order.
sub serialize_records
{
	my ($records) = @_;
	my @blocks;

	for my $record (@{$records})
	{
		my @lines;
		for my $field (@RECORD_FORMAT)
		{
			my $tag = $field->{tag};
			next if $field->{optional} && $record->{$tag} eq '';
			push @lines, "$tag\t$record->{$tag}";
		}
		push @blocks, join("\n", @lines);
	}

	return @blocks ? join("\n\n", @blocks) . "\n" : '';
}

# Parse and validate tagged map records, reporting every fault the file holds
# and returning the source content beside them, so a caller comparing the file
# against a rewrite reads it once.
sub read_map_records
{
	my ($path) = @_;
	my $content = read_utf8_file($path);
	my $records_content = $content;
	$records_content =~ s/\n+\z//;
	return { records => [], faults => [], content => $content }
		if $records_content eq '';

	my @pieces = split(/(\n\n+)/, $records_content, -1);
	my (@records, @faults);
	my %seen_keys;
	my $record_line = 1;

	while (@pieces)
	{
		my $block = shift @pieces;
		my $separator = shift @pieces // '';
		my (%record, %tag_lines);
		my @actual_tags;
		my $line_number = $record_line - 1;
		$record_line += ($block =~ tr/\n//) + ($separator =~ tr/\n//);

		for my $line (split(/\n/, $block, -1))
		{
			$line_number++;
			my ($tag, $value) = $line =~ /\A([A-Z])\t(.*)\z/s;
			if (!defined $tag)
			{
				push @faults, "$path:$line_number: malformed line; write one uppercase tag, a tab, then its value";
				next;
			}
			$tag_lines{$tag} = $line_number;
			push @faults, "$path:$line_number: $tag holds a raw tab or carriage return; write \\t or \\r"
				if $value =~ /[\t\r]/;
			push @faults, "$path:$line_number: $tag opens or closes with a space; write \\s at that edge"
				if $value =~ /\A | \z/;
			push @faults, "$path:$line_number: unsupported $tag tag; write only " . record_tag_order()
				if !exists $RECORD_TAG_MAP{$tag};
			push @faults, "$path:$line_number: duplicate $tag tag"
				if exists $record{$tag};
			$record{$tag} = $value;
			push @actual_tags, $tag;
		}

		if (!exists $record{K})
		{
			push @faults, "$path:$line_number: record is missing K";
			next;
		}
		push @faults, "$path:$tag_lines{K}: duplicate K $record{K}"
			if exists $seen_keys{$record{K}};
		$seen_keys{$record{K}} = 1;
		push @faults, "$path:$tag_lines{K}: invalid K $record{K}"
			if $record{K} !~ /\A[1-9][0-9]*\z/;

		my @expected_tags = map { $_->{tag} }
			grep { !$_->{optional} || ($record{$_->{tag}} // '') ne '' }
			@RECORD_FORMAT;
		push @faults, "$path:$tag_lines{K}: K $record{K} holds "
			. join(' ', @actual_tags) . ' where it takes '
			. join(' ', @expected_tags)
			if join("\x1f", @actual_tags) ne join("\x1f", @expected_tags);

		$record{lines} = { %tag_lines };
		push @records, \%record;
	}

	return {
		records => \@records,
		faults => \@faults,
		content => $content,
	};
}

# Read a program-owned map. The program writes that file, so a fault in it
# names a defect rather than model output an operator or session can repair.
sub read_manifest_records
{
	my ($path) = @_;
	my $result = read_map_records($path);

	die join("\n", @{$result->{faults}}) . "\n" if @{$result->{faults}};

	return $result->{records};
}

1;
