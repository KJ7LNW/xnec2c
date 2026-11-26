package Xnec2c::PO::CatalogState;

use strict;
use warnings;
use utf8;

use Exporter qw(import);
use Locale::PO;

our @EXPORT_OK = qw(
	apply_translation_records catalog_annotation_records catalog_change_records
	catalog_exemption_records catalog_identity catalog_path catalog_plural_forms
	exempt_marker load_catalog po_identity
);

my $EXEMPT_MARKER = 'xnec2c-exempt';
my $CLASSIFIER_PATH = 'scripts/po/po-classify.awk';

# Classifier states that make an entry output-map work.
my %WORK_STATES = map { $_ => 1 } qw(fuzzy untranslated identical);

# Name the authoritative catalog one language owns.
sub catalog_path
{
	my ($lang) = @_;
	return "po/$lang.po";
}

# Join decoded context and source text into one catalog entry identity.
sub catalog_identity
{
	my ($context, $source) = @_;
	return join("\x04", $context, $source);
}

# Derive the named catalog identity fields through Locale::PO.
sub po_identity
{
	my ($po) = @_;
	my $context = Locale::PO->dequote($po->msgctxt);
	my $source = Locale::PO->dequote($po->msgid);

	$context = '' if !defined $context;
	die "catalog entry has no msgid\n" if !defined $source;

	return {
		context => $context,
		source => $source,
		identity => catalog_identity($context, $source),
	};
}

# Reject a repeated identity within the caller's catalog selection.
sub record_unique_identity
{
	my ($path, $seen, $identity) = @_;
	my $key = $identity->{identity};

	die "$path: duplicate catalog identity for msgid $identity->{source}\n"
		if exists $seen->{$key};
	$seen->{$key} = 1;
}

# Load one catalog with the required encoding contract.
sub load_catalog
{
	my ($path) = @_;
	my $catalog = Locale::PO->load_file_asarray($path, 'utf8');

	die "$path: Locale::PO failed to load catalog\n" if !defined $catalog;

	# po-refresh.sh strips obsolete entries from every catalog it writes, so
	# one reaching here means the catalog came from an unstripped path.
	my $obsolete = grep { $_->obsolete } @{$catalog};
	die "$path: $obsolete obsolete entries; run make po-refresh\n" if $obsolete != 0;

	return $catalog;
}

# Return the Plural-Forms value one language's catalog header declares.
sub catalog_plural_forms
{
	my ($lang) = @_;
	my $path = catalog_path($lang);
	my ($header) = grep { po_identity($_)->{identity} eq catalog_identity('', '') }
		@{load_catalog($path)};

	die "$path: catalog holds no header entry\n" if !defined $header;

	my ($plural_forms) = Locale::PO->dequote($header->msgstr)
		=~ /^Plural-Forms:[ \t]*(.*?)[ \t]*$/m;
	die "$path: header entry declares no Plural-Forms\n"
		if !defined $plural_forms;

	return $plural_forms;
}

# Read every syntactically valid exemption name from one translator comment.
sub exemption_comment_names
{
	my ($comment) = @_;

	return () if !defined $comment;
	return $comment =~ /^\Q$EXEMPT_MARKER\E:[ \t]*([a-z]+)[ \t]*$/mg;
}

# Expose the marker spelling every catalog reader and writer shares.
sub exempt_marker
{
	return $EXEMPT_MARKER;
}

# Read every entry carrying an exemption-like translator comment. The audit
# receives representation facts here and applies exemption policy itself.
sub catalog_annotation_records
{
	my ($lang) = @_;
	my @records;

	for my $po (@{load_catalog(catalog_path($lang))})
	{
		my $comment = $po->comment;
		next if !defined $comment;

		my @markers = $comment =~
			/^(\Q$EXEMPT_MARKER\E(?::|[ \t]|$)[^\n]*)$/mg;
		next if @markers == 0;

		my @names = exemption_comment_names($comment);
		my $identity = po_identity($po);
		my $translation = Locale::PO->dequote($po->msgstr);

		$translation = '' if !defined $translation;
		push @records, {
			identity => $identity->{identity},
			context => $identity->{context},
			source => $identity->{source},
			translation => $translation,
			line => $po->loaded_line_number,
			markers => [@markers],
			names => [@names],
		};
	}

	return \@records;
}

# Read every accepted exemption the named catalogs record. The workflow names
# only catalogs preceding the language whose output receives these facts.
sub catalog_exemption_records
{
	my ($languages) = @_;
	my @records;

	for my $lang (@{$languages})
	{
		for my $record (@{catalog_annotation_records($lang)})
		{
			next if @{$record->{names}} != 1;

			push @records, {
				context => $record->{context},
				source => $record->{source},
				exempt => $record->{names}[0],
			};
		}
	}

	return \@records;
}

# Index one catalog by the entry identity a map record names.
sub catalog_by_identity
{
	my ($path, $catalog) = @_;
	my %entries;
	my %seen;

	for my $po (@{$catalog})
	{
		my $identity = po_identity($po);
		record_unique_identity($path, \%seen, $identity);
		$entries{$identity->{identity}} = $po;
	}

	return \%entries;
}

# Read the states po-classify.awk records for one catalog that make an entry
# output-map work, as the set of states each entry identity carries.
sub catalog_entry_states
{
	my ($path) = @_;
	my %states;

	open my $records, '-|', 'awk', '-f', $CLASSIFIER_PATH, $path
		or die "$path: unable to run $CLASSIFIER_PATH: $!\n";
	binmode $records, ':encoding(UTF-8)';
	while (my $record = <$records>)
	{
		chomp $record;
		my ($state, undef, $key) = split /\t/, $record, 3;
		next if !exists $WORK_STATES{$state};

		# The key holds the raw quoted content of msgctxt and msgid; dequote
		# each as po_identity dequotes the loaded entry.
		my ($context, $source) = split /\x04/, $key, 2;
		my $identity = catalog_identity(Locale::PO->dequote(qq("$context")),
			Locale::PO->dequote(qq("$source")));
		$states{$identity}{$state} = 1;
	}
	close $records;
	die "$path: $CLASSIFIER_PATH failed (wait status $?)\n" if $? != 0;

	return \%states;
}

# Select every singular entry po-classify.awk records as fuzzy, untranslated,
# or identical in current catalog state, in catalog order.
sub catalog_change_records
{
	my ($lang) = @_;
	my $path = catalog_path($lang);
	my $catalog = load_catalog($path);
	my $states = catalog_entry_states($path);
	my @changes;
	my %seen;

	for my $po (@{$catalog})
	{
		# Output maps carry no plural forms; the gate projection holds every
		# plural entry, so po-check.sh judges its forms.
		next if defined $po->msgid_plural;

		my $identity = po_identity($po);
		my $entry_states = $states->{$identity->{identity}};
		next if !defined $entry_states;

		record_unique_identity($path, \%seen, $identity);
		push @changes, {
			context => $identity->{context},
			source => $identity->{source},
			copied => exists $entry_states->{identical} ? 1 : 0,
		};
	}

	return \@changes;
}

# Set an unquoted translation while preserving literal backslashes, and record
# the entry's exemption state without replacing unrelated translator comments.
sub set_msgstr
{
	my ($po, $record) = @_;
	my $target = $record->{target};
	my $comment = $po->comment;
	my @comments = defined $comment ? split(/\n/, $comment, -1) : ();

	@comments = grep {
		!/^\Q$EXEMPT_MARKER\E:[ \t]*[a-z]+[ \t]*$/
	} @comments;
	push @comments, "$EXEMPT_MARKER: $record->{exempt}"
		if exists $record->{exempt};

	# Double each backslash so Locale::PO stores literal backslashes in msgstr.
	$target =~ s/\\/\\\\/g;
	$po->msgstr($target);
	$po->fuzzy(0);
	$po->comment(@comments ? join("\n", @comments) : undef);
}

# Write every matched target into the loaded catalog, clearing its fuzzy state.
# An identity the current catalog no longer holds is retired: it is reported
# and omitted rather than failing the transaction.
sub apply_translation_records
{
	my ($path, $catalog, $translations) = @_;
	my $entries = catalog_by_identity($path, $catalog);
	my @retired;
	my $applied = 0;

	for my $identity (sort keys %{$translations})
	{
		my $record = $translations->{$identity};
		my $po = $entries->{$identity};
		if (defined $po)
		{
			set_msgstr($po, $record);
			$applied++;
		}
		else
		{
			push @retired, "$path: WARN K $record->{key} names an entry this"
				. ' catalog no longer holds; omitted from this transaction';
		}
	}

	return { applied => $applied, retired => \@retired };
}

1;
