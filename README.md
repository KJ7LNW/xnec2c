Xnec2c: Fast Multi-Threaded EM Simulator based on NEC2
------------------------------------------------------
**1\. Introduction:**  

[Xnec2c](https://www.xnec2c.org/) is a GTK3-based Graphical version of nec2c, my translation to the C language of NEC2, the FORTRAN Numerical Electromagnetics Code commonly used for antenna simulation and analysis. The original nec2c is a non-interactive command-line application that reads standard NEC2 input files and produces an output file with data requested by "commands" in the input file. In contrast xnec2c is a GUI interactive application that (in its current form) reads NEC2 input files but presents output data in graphical form, e.g. as wire frame drawings of the radiation pattern or near E/H field, graphs of maximum gain, input impedance, VSWR etc against frequency and simple rendering of the antenna structure, including color code representation of currents or charge densities. These results are only calculated and drawn on user demand via menu items or buttons, e.g. xnec2c is interactive and does not execute NEC2 "commands" in batch style as the original does. Printing of results to an output file has been removed starting from version 1.0, since xnec2c works in a way that does not allow printing compatible with the NEC2 format. If printing to file is needed then it is better to use the original NEC2 program, to avoid bugs that may still be lurking in the C translation.

Xnec2c now has a built-in editor for NEC2 input files which can be used to edit geometry or command "card" data. This basic editor displays comment, geometry and command cards in tree views where individual rows, each representing a card, can have their cells edited directly for "raw" entry of data. More useful are pop-up "editor" windows that open when appropriate buttons are clicked or when a selected row is right-clicked with the mouse. These editors allow easier, more convenient entry and editing of individual rows, with no need for detailed knowledge of "card" formats. When editing is completed, the contents of the nec2 editor can be saved in a NEC2-compatible input file which can then be re-loaded by xnec2c for execution.

**2\. Compiling and Documentation:**

See the documentation on the [Xnec2c website](https://www.xnec2c.org/).

**3\. Features:**
*   **Multi-threading operation on SMP machines:**  
    Since version 1.0, xnec2c can run multi-threaded (by forking) on SMP machines, when executing a frequency loop. Multi-threading is enabled by using the -j<n> option, where n is the number of processors in a SMP machine. xnec2c will spawn n child processes, to which it will delegate calculation of frequency-dependent data for each frequency step. Thus data related to n frequency steps will be calculated concurrently and passed on the the parent process by pipes, to be further processed for graphical display. Child processes are spawned before GTK is initialized and started so that only the parent process is tied to the GUI interface. Thus there are n+1 processes running when the -j option is used and execution is faster by slightly less than n times. **Please note** that its pointless and counter-productive to specify a value of n greater than the number of steps in the frequency loop.

*   **On-demand Calculation:**  
    Since xnec2c collects data to be displayed in buffers directly from the functions that produce them, there is no need to produce and parse an output file and no need to re-run the program when certain input data (currently the frequency) is changed or when different output data (gain, near-fields, input impedance etc) is required. The frequency can be changed either from spin buttons in the [Main](https://www.xnec2c.org/#MainWindow) and [Radiation Pattern](https://www.xnec2c.org/#RadiationWindow) windows or by clicking on the [Frequency Data](https://www.xnec2c.org/#FrequencyWindow) window's graph drawing area. The frequency corresponding to the pointer position will then be used to re-calculate whatever data is on display.
    
*    **Built-in NEC2 input file editor:**  
    Xnec2c has a built-in editor for NEC2 input files. Data in NEC2 "cards" can be entered or edited either directly in the main editor window (tree view) or in more convenient dedicated editors for each type of card. Edited data can be saved to a NEC2 input file and reloaded for execution so that the edit-execute-display cycle is quicker and more convenient.
    
*   **Support for Accelerated Linear Algebra libraries: ATLAS, OpenBLAS+LAPACKe, Intel MKL**
    Support for accelerated libraries was added in version 4.3.  Accelerated math libraries such as ATLAS, OpenBLAS and Intel MKL can speed up xnec2c EM simulations if available on your platform.  Library detection details are available in the terminal. See File->Math Libraries->Help for more information. Accelerated operation is optional, it will fall back to the original NEC2 algorithms if necessary.  Accelerated library support has been tested on Ubuntu 16.04, 18.04, 20.04, Debian 9, Debian 11, CentOS 7, and VOID Linux.  Generally speaking, if you can install the requisite libraries, it will be detected.  If libraries are not detected on your OS then please open a bug report.

*   **Interactive Operation:**  
    Xnec2c is interactive in its operation, e.g. when started it just shows its Main window in a "blank" state, indicating that no valid input data has been read in yet. The NEC2-type input file can be specified at start-up in the command line optionally with the **\-i option** or it can be opened from the file selection dialog that appears via the **File->Open menu** of the [Main window](https://www.xnec2c.org/#MainWindow). Once a valid input file is opened, all the normal widgets in the [Main window](https://www.xnec2c.org/#MainWindow) appear so as to allow proper operation. The NEC2 "commands" in the input file are read in but not executed, until a request is issued by the user via buttons or menus in the appropriate windows.
    
*   **User Interface:**  
    In its current form, xnec2c has three windows for the graphical display of output data: When started without an input file specified optionally by the -i <input-file> option, the [Main window](https://www.xnec2c.org/#MainWindow) opens with most of the button and menu widgets hidden. When a valid input file is opened, all the hidden widgets are shown and the structure is drawn in the [Main window's](https://www.xnec2c.org/#MainWindow) drawing area widget. From the **View** menu, the [Radiation Pattern](https://www.xnec2c.org/#RadiationWindow) and [Frequency Related data](https://www.xnec2c.org/#FrequencyWindow) display windows can be opened, to draw either the Gain pattern or the Near E/H fields or Frequency-related Data like Input Impedance, VSWR, Max gain, F/B Ratio, Gain in the Viewer's direction etc. Both the [Main window](https://www.xnec2c.org/#MainWindow) and the [Radiation Pattern window](https://www.xnec2c.org/#RadiationWindow) have buttons to select fixed viewing angles of the structure or the radiation pattern, as well as spin buttons to input specific viewing angles.

*   **Color Coding:**  
    Xnec2c uses color coding to visualize the Current or Charge distribution in the Structure's segments or patches as well as the Gain pattern or the Near E/H field pattern. Color coding is also used to clarify the Graphs of Frequency-related data. A color code strip is shown in the [Main](https://www.xnec2c.org/#MainWindow) and [Radiation Pattern](https://www.xnec2c.org/#RadiationWindow) windows.

*   **OS Support** 
    Xnec2c is known to build on many different platforms including Linux and [FreeBSD FRESH ports](https://www.freshports.org/comms/xnec2c/).  If you have trouble building on your OS then please open a bug report.
    
*    [Click here to view the full documentation](https://www.xnec2c.org/) or browse doc/xnec2.html in this git tree.


**Acknowledgment:**  
Some of the code in xnec2c is based on Pieter-Tjerk de Boer's "xnecview" application, which visualizes NEC2's input and output files. The general 'look and feel" of xnec2c was also influenced by the same application.

**Author:** Neoklis Kyriazis (Ham Radio call: 5B4AZ)
