# Widget state and config hooks

Apply each line when writing widget values programmatically, changing signal handlers or reset sequences, disabling a widget, branching on widget state, or adding config item hooks.

## User-given

- Programmatic widget writes: isolate via SIGNAL_BLOCK/SIGNAL_UNBLOCK (src/callbacks.h); boolean suppression flags and conditional guards in signal handlers are prohibited
- Reset sequence: write defaults → invoke post-apply hooks → block signals → sync widgets → unblock; signal handlers serve user interaction only
- Must add tooltips need to describe the reason for any disabled (eg, "greyed-out") mechanism/UI widget, so the user knows why it's not available. 
- Prefer branching on engine or deck data instead of user interface widgets: User interface widgets are usually dependent on engine or deck definitions. So we must always use the single point of truth in the tree, not a dependent source that duplicates information and creates the potential for a break in chain of custody. 

### config / rc_config: 

Any modification hooks necessary against a configuration item needs to set `.widgets = CONFIG_WIDGET_TREE( .pre/post_apply =...)`
