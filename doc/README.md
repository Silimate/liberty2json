# Synopsys Liberty Parser

Please, refer to COPYING.pdf for the license.

## Compilation Instructions

Ensure that `cmake` and `make` are installed. Then run:

```
rm -rf build
mkdir build
cd build
cmake ..
make -j8
```

The binary will be located at `build/parse_liberty`.

## Copyright

Copyright (c) 1996-2005 Synopsys, Inc.    ALL RIGHTS RESERVED

This package is contains various tools for working with
liberty, a standard developed by Synopsys for describing cell libraries.

## Authors

For questions or comments please visit the Open Source Liberty page 
(http://opensourceliberty.org/cgi-bin/contactus.cgi).

## Dependencies

To build this package, you will need, as a minimum, the following
utilities. They may be substituted with local favorites, hopefully
with little tweaking of the makefiles or code:

- bison  version 1.875  (1.22 will not work; 1.28 does)  and up. Earlier versions than
       1.875 will work fine, except syntax errors generate slightly different messages,
       which cause some regression tests to fail.
- flex   version 2.5.2 and up.
- gperf  version 3.0    (2.5 will not work-- some OK .lib files will not pass check)
- gcc    version 2.8.1, egcs 2.91.60, 2.95.2, or above

Optionally useful, perhaps required:

- GNU make   version 3.79.1  and up... ** non-gnu makes probably won't work! **
- diffutils  version 2.7  if your system can't handle "diff -q ..."
- sh-utils   version 2.0  if your echo can't handle the -n option...
- libtool    version 1.4.2 ;; most users will not need need this unless
                            seriously wanting to change things in 
                            developer mode.

You can pick up GNU software from:

(an up-to-date listing of ftp sites is at:
	http://www.gnu.org/order/ftp.html )

- gatekeeper.dec.com/pub/GNU
- ftp.keystealth.org/pub/gnu
- ftp.itcentrx.com/pub/gnu
- uiarchive.cso.uiuc.edu/pub/ftp/ftp.gnu.org/gnu
- ftp.cs.columbia.edu/archives/gnu/prep



Other versions may be used, both older or newer. The above versions 
"work"; others may or may not "work".



NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW

A good forum for discussions about liberty and Physical Liberty (plib) is at:

http://synopsys.infopop.net/OpenTopic

Bugs and enhancement requests and pleas for aid can be posted there, as well as late-breaking news.

NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW--NEW

Included in this distribution are the files PI.c and si2dr_liberty.h, an
implementation of a procedural interface modeled after the SI2DR
interface (see www.si2.org or www.edaopen.org). This PI is the
foundation of this toolkit, and provides the capability of building,
traversing, reading, writing, and modifying liberty data. It presents
64 functions (to date).

The excutable, syntax_parse, will read the Synopsys supplied files in
the desc directory, and generate a series files that will be compiled and
linked into the program liberty_parse.

The executable, liberty_parse, will read a liberty file, and then
perform a set of syntax/semantic checks on the data.

Using the including PI, other applications can be swiftly built around
the liberty file standard.

The main goal of this project was to make the code ridiculously simple
to maintain; it's all written in C as simply and straightforwardly as
I could manage, and uses no runtime interfaces beyond the command line.

The syntax is very simple: You have groups, attributes (attrs), and
define statements. Attrs can be complex or simple. Groups can have
several names. The parser could not care less about group names, attr
names, or types. The Checking routine will report all unrecognized
group names and attr names to make sure they are allowed within the
context of the technology and timing models declared.

The reason for this kind of seperation of the syntax from the
semantics was so that future enhancements and upgrades should be very
easy to implement in this toolkit as liberty grows and matures.

The syntax/semantic checking is built on a foundation of syntax
description files that are provided by Synopsys. I have placed a set
of these in the desc directory. If extensions or changes are mode to
liberty, these files can be updated, and the parser regenerated with
"make".

From these syntax description files, a series of automatic checks can
be made. The rest of the checks are hand coded. Not all possible
checks have been coded to date. It is hoped that the user community
will help by contributing to this effort.

### Syntax/semantic checks

#### Automatic checks

Using the files in desc/, these checks are performed:

1. Group names are correct.

2. The number of names included in the
   parenthesis are correct (some groups do not allow a name, others have
   a single name, others have an optional name, others have one or more
   names, etc.)

3. Group ownership is checked. Not every group may contain a certain group.

4. attribute ownership is checked. Each group is allowed to contain a
   specific subset of the attributes.  

5. Attribute types (simple vs complex) are checked.  

6. Attribute value types are checked (string vs. float vs. boolean
   vs. int, etc).  

7. Attribute value ranges (where specified) are checked.  

8. name space (ie. scoping) rules are checked. It is illegal to
   declare two similar group types with the same name.  Some group
   names are really references; these have to be excluded from this
   check by hand, as the distinction is not made in the syntax descriptions. 

#### Hand-coded checks

1. table checks: the index, value specs in timing groups vs. templates are made. 
   The code will indicate if the wrong number of entries are present.

2. It will verify that "members", if present, is the first entry.

3. if both "clear" and "preset" attrs are given, it will check for
   "clear_preset_var1" and/or "clear_preset_var2".

4. pin/bus direction compatibility is checked.

5. pin&bus driver_types are checked to make sure they are compatible
   with each other and with pin direction.

6. timing subgroups are checked for compatibility.

7. the delay_model and technology are checked to insure that the
   delay_model is specified first.

8. each bus must have a "bus_type" attribute, that references
   an existing "type".

9. The index_xxx attributes must not have any negative numbers, or
   an error is given.


### Regression tests

Simple, small regression tests exist in the test/ subdirectory. The
script, "runtests" is provided to run them. Each of the previously
mentioned checks should be checked at least once by the regression
tests, both for success and failure. They are very useful in
demonstrating weaknesses in the code, and keeping old bugs from being
resurrected.

### Files:

Here is a list of files (with some description) that come with the distribution:

- PI.c                    - main file for the si2dr_liberty interface
- libhash.c               - hashing functions used all over the place
- libstrtab.c             - a string table implementation that uses libhash.
- main.c                  - the main func for liberty_parse
- mymalloc.c              - malloc front ends for error checking
- syntax_checks.c         - the semantic checks are almost all in this file
- syntform.c              - functions to generate code from the syntax description files
- liberty_parser.y        - yacc source for the liberty parser
- syntax_parser.y         - yacc source for the desc files code generator
- synttok.l               - lex source for the desc files code generator
- token.l                 - lex source for the liberty parser
- liberty_structs.h       - structures used in PI.c
- libhash.h               - definitions for the hash package
- libstrtab.h             - definitions for a string table implementation built on libhash.
- mymalloc.h              - definitions for the malloc front end
- si2dr_liberty.h         - The Official .h for the si2dr_liberty interface
- syntax.h                - definitions used in the desc files code generator
- desc/                   - this directory holds syntax description files for different techs.
- desc/syntax.cmos.desc
- desc/syntax.fpga.desc
- AUTHORS                 - a list of contributors
- COPYING.pdf             - the SYNOPSYS Open Source License Version 1.0
- ChangeLog               - a list of changes made to files
- INSTALL                 - installation instructions.
- Makefile.am             - used to build makefiles
- Makefile.in             - used to build makefiles
- NEWS                    - news about the state of the program
- README                  - useful for learning many things
- acinclude.m4            - used to make makefiles, configure scripts, etc
- aclocal.m4              - used to make makefiles, configure scripts, etc
- bnf
- configure.in            - used to make the configure script
- config.sub              - used in the configure process
- config.guess            - used in the configure process
- configure               - run this to configure and generate makefiles
- install-sh              - used in the install process
- ltconfig                - used in the configure process -- in version 1.4 & up, this is no longer necc.
- ltmain.sh               - used in the configure process
- missing                 - needed to build makefiles
- mkinstalldirs           - used in the install process
- si2dr_liberty-PI.txt    - documentation for the si2dr_liberty interface
- test/                   - this directory hold the regression tests
- test/runtests           - the script to run all the tests (also, make check)
- ylwrap                  - a wrapper to call yacc/lex/bison/flex

## Change Log

Change list for liberty parse 2.6

### Improvements/Support:
-	Added support for transition degradation groups in liberty_get_value function.
-	Added support for rise/fall propagation in liberty_get_values.
-	Added additional checks for complementary_pin attribute.
-	Added support for data retrieval from CCS noise groups
-	Added support for multiple power_rail attributes.
-	Added support for retention cell/pin syntax.
-	Added support for reading/writing input driver waveform into liberty files.
-	 Added support for multi voltage_map.
-	Added support for define_group output .
-	Added support for multiple intrinsic_resistance groups.
-	Added support for variation aware syntax.
-	Added support for multiple va_compact_ccs_rise/fall va_receiver_capacitance1/2_rise/fall va_rise/fall_constraint group attribute.
-	Added support for for retaining_fall/retaining_rise group. 
-	Added support for attributes for switch cell (MTCMOS).
-	Added support support for multiple normalized_driver_waveform .
-	Added support for retain_rise_slew/ retain_fall_slew in  liberty_get_values_data function .
-	Add support for is_unbuffered attribute. 

### Bug fixes:
-	Fix is to modify the liberty writer to use %g when printing library attributes: MAXX and MAXY.
-	Memory errors and leaks.
-	Recognize normalized_driver_waveform group when retrieving values using liberty_get_values_data

### Liberty format changes:
* 1.	Lib has new attribute(s)
   * a.	switch_cell_type  = string ("coarse_grain")
* 2.	cell ->pg_pin has attributes 
   * a.	direction : enum(inout, input, output or tristate)
   * b.	switch function : virtual_atrribute
   * c.	pg_function :  virtual_atrribute
   * d.	std_cell_main_rail : boolean
* 3.	cell ->bundle subgroup has two new attributes:
   * a.	pull_up_function: virtual_atrribute
   * b.	pull_down_function: virtual_atrribute
* 4.	cell->internal_power group changed 
   * a.	pg_pin type from "virtual_attribute" to "string"
* 5.	cell->pin group has added/changed new attributes
   * a.	pull_up_function: virtual_atrribute
   * b.	pull_down_function: virtual_atrribute
   * c.	power_down_function: virtual_atrribute
   * d.	switch_pin: boolean
   * e.	input_voltage_range(float, float)
   * f.	output_voltage_range(float, float)
   * g.	retention_pin(string, string) changes to retention_pin(string, int)
* 6.	cell->pin->receiver_capacitance  subgroup  has added new attribute
   * a.	when : : virtual_atrribute
* 7.	cell->pin->pin_based_variations subgroup has added new attribute
   * a.	when : : virtual_atrribute
   * b.	
* 8.	cell->bus has added new attributes
   * a.	pull_up_function: virtual_atrribute
   * b.	pull_down_function: virtual_atrribute
   * c.	compact compact_ccs_power ( [compact_ccs_power_template_name] ) {
                   index_output : string; 
                   values( <unknown_arguments> );
* 9.	test_cell has added new subgroup:
       dc_current( [dc_current_name] ) {
           related_switch_pin      : virtual_attribute;
           related_pg_pin          : virtual_attribute;
           related_internal_pg_pin : virtual_attribute;
           index_1(list);
           index_2(list);
           values( <unknown_arguments> );
        }

## News

Version 2.5
1. Syntax changes:
    a. voltage map added to library level
    b. pg_pin added to the cell level
    c. added gate_leakage under leakage_current
	d. add input_signal_level_high and input_signal_level_low attrs under pin level
2. support for multiple power_rail, voltage_map attributes added;
   also multiples of variation-aware stuff: 
    a. va_compact_ccs_rise
    b. va_compact_ccs_fall
    c. va_receiver_capacitance1_rise
    d. va_receiver_capacitance1_fall
    e. va_receiver_capacitance2_rise
    f. va_receiver_capacitance3_fall
    g. va_rise_constraint
    h. va_fall_constraint
3. print several quantities in exponential format 
    a. va_values and nominal_va_values
    b. input_low_value and input_high_value in gate_leakage
    c. value in pg_current and intrinsic_capacitance
4. support output of define_group elements.
5. allow multiple libraries to be read in.
6. allow multiple intrinsic_resistance groups 
7. fix for a crash on SunOs  (null File name)
8. Allow retrieval from lu_table_templates from CCS noise groups
9. get template name for:
   va_receiver_capacitance2_rise
   va_receiver_capacitance2_fall
   va_receiver_capacitance1_rise
   va_receiver_capacitance1_fall
   va_fall_constraint
   va_rise_constraint
10. fix a possible crash with an uninitialized/NULL char pointer 
    in liberty_get_index_info(), and in get_index_info() in 
    check_syntax.c.
11. search for templates for retaining_fall/retaining_rise group
    (the overwhealming majority of these fixes were contributed by
     folks at Synopsys! Many Thanks!!!!)
12. Code cleanups (prototypes added for the sake of gcc warning messages), 
    some reformatting, etc.
13. a fixed memory-leak situation in syntax_checks.c, routine 
	check_group_correspondence(). Many thanks to Roman for his
   	reporting this issue and suggestions as to a fix!
14. Updated the automake support scripts for 1.9
15. A fix for boolean defines submitted by Albert Ma (via the
    infopop web site). Many thanks to Albert! (booleans were
    being dropped).
16. In preparation for a perl interface, I added a series of functions
    that allow si2drExprT to be opaque. So, now you have functions
    that allow you to create and exract data from the ExprT's, as if
    it were an opaque type,  and you also can just use the pointer 
    to manipulate the fields (as before).
17. The DefineGetInfo func provided some problems for a perl interface,
    so 3 new functions were added to the interface to pull out seperately
    the 3 fields that the DefineGetInfo function returned. (DefineGetName,
    DefineGetAllowedGroupName, DefineGetValueType). The DefineGetInfo
    function will not be available to the perl interface, but the 3 new
    functions will provide the same functionality.
18. A new object type, si2drAttrComplexValIdT, basically to represent
    the struct of a complex attribute, is introduced to allow
    new functions to access each of the attributes separately, which
    makes a perl interface much simpler. (Probably other interfaces, too).
    The old methods (si2drIterNextComplexValue) still works; the new
    alternative method is to use si2drIterNextComplex(), and 
    si2drComplexValGetValueType(), si2drComplexValGetInt32Value(),
    si2drComplexValGetFloat64Value(), and etc. to get the same effect.
18. To generate the perl interface, type "make perl/liberty.so".
    (make sure you have swig-1.3.31). The .pm and _wrap.c files
    will be provided with the release, so you don't really need
    to have swig to use the perl interface.
19. To run a simple test of the perl interface, type "cd perl; ./t1.pl"
20. Some small improvements to si2drExprToString().
21. The parser was originally built to turn all incoming numbers in a lib
    file into a floating point representation. This would show up in 
    output files where what went in was an integer, and what comes out
    is a floating point number. So I fixed up the lexer and parser
    to preserve this information. 
22. I also restored a check to verify that a defined attr is of the 
    proper type. I had previously coded part of this check, 
    and subsequently commented it out. If it gets too irritating, 
    let me know, perhaps I can provide a command line option
    to turn this check on or off.
23. New additions to syntax:
    a. related_power_pin : string; added to pin
    b. related_ground_pin : string; added to pin
    c. related_pg_pin : string; added to leakage_power and internal_power
    d. level_shifter_type : (enumerated value) added to cell
    e. input_voltage_range(float,float) added to cell
    f. output_voltage_range(float,float) added to cell.
    g. std_cell_main_rail: boolean added to pg_pin
    h. retention_cell : string added to cell
    i. retention_pin(str, str)    added to pin.
24. The retention pin checks added:
	a. not allowed: retention_cell and power_gating_cell on same cell
    b. not allowed: retention_pin and power_gating_pin on same pin
	c. retention_pin cannot be defined on an output pin
    d. map_to_logic and retention_pin together: warning.
25. Level shifter checks added:
	a. level_shifter_type, input_voltage_range, output_voltage_range
       are only allowed in a cell, if is_level_shifter is true.
    b. input_voltage_range and output_voltage_range must both appear in a cell or not at all.
26. new regressions added to the test dir, both to debug the new code
    added, and insure that future changes will not violate these additions.
27. User defined groups will generate an error if instantiated without a group name.


Version 2.4

1. Syntax changes:
   a. in bundle and pin, the power_gating_pin arguments are now string, int
   b. ccsn_first_stage, ccsn_last_stage  added to pin
   c. compressed_ccs_timing_rise, ccsn_first_stage, ccsn_last_stage added to timing, and bus
   d. several constructs added to output_current_rise in timing.
   e. ccs_timing_base_curve_template, ccs_timing_base, ccs_lu_table_template  added to library
   f. several updates made to parser to handle 64-bit values.
   g. multiple define_group attrs and curve_y attrs now allowed.
   h. some uninitialized variable bugs fixed in PI.c and check_syntax.c

   


Version 2.3

1. Many thanks to the Synopsys Tap-in user chrisj, who found and supplied
   a patch for the GroupCreateAttr and GroupCreateDefine funcs, which were
   assigning the passed-in pointer for the name, instead of the strtable entry 
   for it. I've applied the patch and gave him credit in the source.
2. Invented tests/t4 to demonstrate the problem (without the fix, a1 and a2 won't be found),
   and also acts as a regression test.
3. A bug was found in the way complex attribute string values are parsed from 
   the values constructs, where it would stop at line continuation syntax.  
   This bug has been fixed. Some mods to the example.ccs testcase to demonstrate
   this problem/solution as a regression test.
4. in liberty_parser.y, and main.c, included missing headers to avoid compiler
   warnings. Several testcase programs were also so upgraded, to avoid compiler
   warnings that would make a mess of the "PASSED" messages.


Version 2.2

1. A small bug in the trace code, where the wrong function name was
   being output, was corrected.
2. The PIQuit() function didn't free up any groups that might have been attached 
   under the PI. Added Code to destroy and free all groups, using ObjectDelete() func.
   This could have been considered a huge memory leak, if you executed multiple
   PIInit() and PIQuit() pairs in a single process run.


Version 2.1

1. The ObjectDelete function wasn't freeing elements as it unlinked them.
   This major memory leak has been plugged.
2. The si2dr_liberty-PI.txt wasn't included in the release tarfile for some
   reason. This has been rectified.

Version 2.0

1. Various improvements to the Makefile.am and configure.ac files, to
   bring them up to date with the current autoconf and automake tools.
2. String Tables are implemented in the PI. There is an average of 10
   lookups to every string put in the string table, which should reduce
   memory usage. A string table is a big chunk of memory in which every
   string is stored. A hash table on top of that ensures that only copy
   of any string is stored in the table. Entering strings into the table
   can overflow the initial allocation of the string table. When this happens,
   a new "chunk" of memory is allocated, and the new strings are then placed
   in this "chunk". The chunks are arranged in a singly linked list, the latest
   chunk always at the front of the list. 
3. The PI has been upgraded to use the String Table. The parser enters all
   incoming strings into the string table, and passes back string pointers
   into the table. The PI enters all incoming strings into the table.All string 
   pointers therefore given to the user by the PI, are from the string table.
   All such strings are freed by the PIQuit call. It is hoped that this
   will simplify using the PI, as memory management of strings is simplified
   greatly thereby.Since all strings passed into the PI via function calls, are 
   copied into the string table, there is no need to malloc these strings; they 
   can even be on the execution stack, for that matter.
4. Some improvements in cleaning up memory leaks were submitted by Synopsys
   personnel. Many thanks! Other leaks, we found/created and fixed ourselves.
5. The CCS syntax has been added to the CMOS syntax description.
6. The following checks, relating to CCS data, have been added:
   (from "NLDM and CCS Library Characterization Guidelines for PrimeTime
          and PrimeTime SI", Version 0.0, by Karen Linser, PrimeTime Corp Appl. Eng.)
	2.3.1 Zero values in vector values.
    2.3.3 Current Polarity
	2.3.4 Peak Current not at beggining of vector
    2.3.5 Current vector variables in template of correct type and position.
    2.3.6 Reference_time not negative
    2.3.7 Identical reference_time vals for fixed transition time
    2.3.9 Library nom_voltage same as default operation condition's voltage val.
    2.6.2 Digits of significance. The code issues an error if any of the vector
          numbers have less than 4 digits of precision.
      ?   A check to verify that CCS output_current_rise/fall vectors
          cover all permutations of the index_1 vs index_2 values used.
          If all permutations are present, the array of vectors is said
          to be "dense".
7. Helper struct/functions added to the PI:

    struct liberty_value_data
    {
    	int dimensions;
    	int *dim_sizes; /* a malloc'd array of <dimensions> numbers,
    					   each number is the size of the array in that dim.*/
    	long double **index_info; /* a Ptr to a malloc'd array of size <dimensions>, ptrs to
    								 malloc'd arrays of long double index values.
        						     Each array of long double is of length set by
    								 the corresponding dim_sizes array values */
    	long double *values; /* a ptr to a malloc'd array of long doubles,
    							 starting with [0,0,...,0,0], and ending with
    							 [z-1,y-1,....b-1,a-1], where a-z are the max
    							 number of elements in each dimension. */
    };

    long double liberty_get_element(struct liberty_value_data *vd, ...);  /* returns NaN if bounds exceeded */

       -- use the function call above to fetch a single entry from the values table.
          for a 3-d table, you'd say "liberty_get_element(vd,1,2,3)"
          the arg order is index_1, index_2, etc. The number returned will be a "NAN"
          if the bounds are exceeded by the reference.

    void liberty_destroy_value_data(struct liberty_value_data *vd);

       -- this will free up all the memory in the liberty_value_data struct.
          

    struct liberty_value_data *get_values_data( si2drGroupIdT table_group);
	
       -- pass this func the oid of a group that has a reference to a template, and
          contains a "values" attribute, and it will traverse the OID's and return all
          the data in a structure, for easier analysis. All floating point numbers
          will be 96-bit "long double" format.
8. Several mods to reduce messages from compiler during a compile with CFLAGS=-Wall.
9. For Solaris machines (and possibly others), the use of "long double" is not practical,
   as special math libs are necessary to provide functions like strtold. I updated the
   INSTALL file to describe CFLAGS options to use on such platforms. I added code to 
   provide non-long-double versions of all such variables when these options are used.
10. Added new PI funcs: 
	si2drStringT   si2drGroupGetComment   SI2_ARGS(( si2drGroupIdT group,
													 si2drErrorT  *err));
	
	si2drVoidT     si2drGroupSetComment   SI2_ARGS(( si2drGroupIdT group,
													 si2drStringT comment,
													 si2drErrorT  *err));
	
    si2drStringT   si2drAttrGetComment      SI2_ARGS(( si2drAttrIdT attr,
													   si2drErrorT  *err));

    si2drVoidT     si2drAttrSetComment      SI2_ARGS(( si2drAttrIdT attr,
													   si2drStringT comment,
													   si2drErrorT  *err));

    si2drStringT   si2drDefineGetComment      SI2_ARGS(( si2drDefineIdT def,
														 si2drErrorT  *err));

    si2drVoidT     si2drDefineSetComment      SI2_ARGS(( si2drDefineIdT def,
														 si2drStringT comment,
														 si2drErrorT  *err));

    The parser has been upgraded to store comments in a buffer, and then use the above 
    functions to attach the comments to whatever construct they precede.
	This implies a forced sort of comment style, but at least it is a common one.

    The Write function has also been upgraded to generate the comments immediately
    preceding the object (group, attr, define) they are associated with. 

    Comments are stored without the opening and closing syntactic markers (/* and */)
    Multi-line comments are stored EXACTLY as specified. All spacing and linefeeds
    will be preserved. This may or may not line up with the indentation provided by
    the Write function.
11. Many thanks to David Mallis of SI2!! He found a bug in the define_group code, and
    provided a fix. Basically, if a new group is defined, and via multiple define_group
    entries, is allowed to be in more than one group, then the syntax checking code
    messes up, as it was written to have a defined group only allowed in one place.
    David's code fixes the error. His test case is now one of the regression tests.
12. Many thanks to Chenzhong Luo of Synopsys for discovering bugs in the MoveAfter 
    and MoveBefore functions. In both functions, the checks for "already done" conditions
    were wrong or missing, which could lead to some nasty things being done to the
    linked list pointers! They are now properly present in the code, and return a success
    value, as the job IS complete. Also, these functions were not documented in the si2dr_liberty-PI.txt
    file, so this was corrected also.


Version 1.15

1. Spotted a crash in an error situation on solaris. Involved writing
   a partially parsed file-- partially because of syntax errors. 
2. Fixed the runtests script because different platforms seem to yield different
   orders of files when * is specified! Turned the glob into an absolute
   list.


Version 1.14

1. Fine tuning the parsing of exprs: VDD and VSS will not be kept as a
   string type simple attr. They will set as exprs. Almost all other identifiers,
   used in a simple attr value definition, will be returned as a simple string value
   instead of an expr...
2. The var.c -> var9 regression test added.
3. Some fixes to avoid crashes when the group OID is null. More still to do.
4. Added a function si2drExprToString, which will take an expr node pointer, and
   return a malloc'd string containing the string representation of that expr.


Version 1.13

1. Users voiced a need to minimize impact of the Exprs on attribute values.
   To do this, I filter exprs as they are put into attribute values. If they are 
   single term, and represent a number or VDD or VSS, they are reset back to simple 
   attribute values. This reduces the number of exprs that appear in the input.




Version 1.12

struct liberty_value_data *get_values_data( si2drGroupIdT table_group);

struct liberty_value_data *get_values_data( si2drGroupIdT table_group);

1. A bug in the syntax check code for busses caused a crash on Solaris; 
   this is now fixed.

2. Haven't been updating the version info. This is rectified in 1.12. Sorry!




Version 1.11

1. Upgraded to now Allow the declaration of variables. Example:  DISAST = 5;
   These are stored as Attrs, but have the newly added attribute of IsVar set,
   to allow differentiation.

2. Now allow colons in group header names.

New interface functions:

si2drBooleanT    si2drSimpleAttrGetIsVar     ( si2drAttrIdT attr,
											   si2drErrorT     *err );
si2drVoidT       si2drSimpleAttrSetIsVar     ( si2drAttrIdT   attr,
											   si2drErrorT      *err );

    Note: Existing user code will have to be modified to differentiate Simple Attrs that
    are Variable declarations, from normal simple Attrs.
    Simple Attrs that have IsVar set are not checked against the syntax templates
    to see if they allowed. In this case, the syntax templates do not apply.

3. Modified the checks for power_poly_template.

4. New regression files:

   example.sppm.lib
   example.var.lib
   example.coloncell.lib
   example.colontempl.lib

   Also, an update to example.include.ref was made to handle new syntax due 
   to adding variable declarations.

5. Forgot to record some changes in 1.10 in the NEWS file. This has been
   rectified.







Version 1.10

1. Modified several things to allow general arithmetic expressions in 
   attribute values. 

   CAVEATS:

   1. They aren't COMPLETELY general arithmetic expressions-- 
   right now, what is supported is parentheses, identifiers, numbers, and these
   operations  (+, -, /, *), along with the prefixes (+, -). Other operations 
   can be added if they are neccessary. These expressions obey C language operator
   precedence.

   2. Syntax/Semantic checks on such expressions which might require 
      evaluation is not done.
	

2. New PI funcs: 
	si2drExprT      *si2drSimpleAttrGetExprValue( si2drAttrIdT attr,
                                                  si2drErrorT  *err );
	si2drVoidT       si2drSimpleAttrSetExprValue( si2drAttrIdT   attr,
                                                  si2drExprT    *expr,
                                                  si2drErrorT   *err );
    si2drVoidT       si2drExprDestroy( si2drExprT   *expr,  /* recursively free the structures */
	                                   si2drErrorT  *err);
    si2drExprT      *si2drCreateExpr( si2drExprTypeT type, /* malloc an Expr and return it */
								  si2drErrorT  *err)

Modified these Functions:

si2drVoidT     si2drIterNextComplexValue         ( si2drValuesIdT iter,
												   si2drValueTypeT *type,
												   si2drInt32T     *intgr,
												   si2drFloat64T   *float64,
												   si2drStringT    *string,
												   si2drBooleanT   *boolval,
												   si2drExprT      **expr,
												   si2drErrorT     *err )
si2drVoidT     si2drIterNextComplexValue         ( si2drValuesIdT iter,
												   si2drValueTypeT *type,
												   si2drInt32T     *intgr,
												   si2drFloat64T   *float64,
												   si2drStringT    *string,
												   si2drBooleanT   *boolval,
												   si2drExprT      **expr,
												   si2drErrorT     *err )

New Types, enums, etc. :

si2drExprTypeT;
si2drExprT;

Modified Types:

si2TypeT;
si2drValueTypeT;






Version 1.9:

1. Added the syntax for added polynomial & table noise specs. See desc/syntax.cmos.desc

2. The OR symbol "|" in boolean expressions like function, next_state was
   not supported by liberty_parse executable:
    ff_bank(IQ,IQN,4) {
        next_state : "(D&EN)|(D&!EN)";
        clocked_on : "CK";
    }
    Error: t.lib:7, This ff_bank, with width 4, references the object |,
    which I cannot find.

   This is repaired.

3. The inputs in the next_state attribute of a ff_bank or latch_bank
   need not be a bus or bundle. If they are of single bit, then they are
   fanned out to all ffs or latches. But the outputs should be of
   bus/bundle type.
     Error: t.lib:7, This ff_bank, with width 4, references the object SE,
     which I cannot find.

     library(ffBank) {
       cell(bank) {
         ff_bank (IQ, IQN, 4){
          clocked_on : "CK" ;
          next_state : "SE" ;
         }
         pin(SE) {
            direction : input;
         }
         pin(CK) {
            direction : input;
         }
         bundle(Q) {
            members(Q0, Q1, Q2, Q3);
            direction : output;
            function  : "IQ";
        }
     }
   This false reporting of an error has been corrected. It checks to see
   if the name referenced is declared as a pin. If so, no error is generated.

4. A request was made to have all messages go thru a registerable print function,
   which users could set, so as to control the output of error, warning, and note 
   messages.

	So, added to the interface is an enum, si2drSeverity, which describes the 3 
	severity levels of output messages.

	A PI-provided Default Message Handler is incorporated into the library, 
    appropriately named si2drDefaultMessageHandler.

	A new function, si2drPISetMessageHandler() is provided to allow users to 
    insert their own function into the message flow. They can set the default 
	message handler back by simply calling this function with the publicly 
    available function as an argument.

	A new function, si2drPIGetMessageHandler() was added so the interface could 
	use the proper function to print out error messages.

	All functions in PI.c were modified to send their messages thru the message
    handler.

	And, all 214 or so printf functions were modified to the new regime in syntax_check.c.

	Plus, some messages in the parser were also converted.

	The default handler is set in the PIInit() function.

	And, finally, every test's 'ref' file had to be updated because of the 
    changes in the output messages.

5. In the hash code, changed a few 'long's to 'int's, so as to make the code work
   on 64 bit platforms.

6. Identifiers now allow ":" (colon) in their contents. Users must beware that
   the colon as a syntactic element must be surrounded by spaces to be recognized. Apparently
   the lc tool from synopsys already demands this, so it should only bring liberty_parse
   into closer alignment with lc.

7. In the si2drWriteLibertyFile  function, for each group, we were generating first, all the 
   defines, then the attrs,  then the subgroups. In so doing, we were generating some attrs out 
   of order, namely for the following attrs, we want to generate them AFTER the group they
   reference.

		
         default_operating_conditions
         default_wire_load
         default_wire_load_selection
      
   So, the code has been altered, and the attrs on this list are now generated AFTER the 
   subgroups.

8. A couple users have expressed the handiness of being able to insert a group anywhere in 
   the list. It seems a reasonable request. So, I have added these funcs to the interface:

      si2drVoidT si2drGroupMoveAfter(si2drGroupIdT groupToMove, 
                                     si2drGroupIdT targetGroup, 
                                     si2drErrorT *err);

      si2drVoidT si2drGroupMoveBefore(si2drGroupIdT groupToMove, 
								      si2drGroupIdT targetGroup, 
                                      si2drErrorT *err);

   Move the "groupToMove" group to the spot either in front of, or behind, the provided
   group. The two groups had better have the same parent!
   To use these, of course, you will have to first Create the group as normal, which
   places it at the end of the list; then move it where you want it.



Version 1.8:

1. Forgot to update the GetErrorText routine to return a string for SYNTAX_ERROR.
   Now Rectified.

2. Fixed GroupAddName for the case where an object has no names at all. Adjusted
   the regression test t3 to spot this in the future so it won't recur.

3. Modified the check for "internal_power", so the check is done only if values
   are present. Added the internal_power check to the enclosed power/rise_power/fall_power
   groups.



Version 1.7:

1. Thanks to Francois Barriere, of Atmel in France, for a fix to ReadLibertyFile,
   which prevents a crash in the yyerror routine.

2. Made a fix to si2drGroupDeleteName(); It would crash if the group is a
   library, or pretty much all the time anyway. I may add a regression test
   which includes the test case I wrote to find the bug.

3. WriteLibertyFile-- String values of complex attributes were not being output 
   enclosed in quotes, which would yield parse errors when the generated file
   was read back in. Fixed now.

4. Added a new Error code to si2drErrorT, "SI2DR_SYNTAX_ERROR", which will be
   returned from LibertyReadFile if syntax errors were encountered in the
   input file. This is set when the yyerror function is called, and zeroed
   at the beginning of the parse... Also, the parser application will output
   a message if the ReadFile func returns this new error code.

5. Added a check to "internal_power" that verifies that the "values" correspond
   to the referenced template and are of the proper groupings and number, just
   as the other lu_template_table checks.



Version 1.6:

1. Thanks to James Cherry, who sent in a list of bugs with fixes. Some real
   whoppers have been in there a long time, but are now corrected.

	a. forgot to set d->owner in si2drGroupCreateDefine, which
       made deleting libraries rather spicy! 

    b. a small truckload of typos in PI.c like "caseSI2DR_DEFINE" which should
       have been "case SI2DR_DEFINE", were fixed. It's amazing this slipped 
       thru as valid syntax, but it was, and created infinite loops in certain 
       situations.

    c. the lex rule to pick up voltage formulas in the input_voltage group was 
       sadly lacking. James supplied a much better rule, which I inserted.

    d. in token.l, a bunch of string duplications were done, malloc-ing only
       strlen() bytes, not strlen()+1 bytes. This is fixed, and the copying
       of a string is now relegated to a function (my_strdup).

    e. Reading multiple files doesn't reset the line numbers. I did as James 
       advised, and reset the line count in the file rule in liberty_parser.y.

    f. fixed a small bunch of warnings in PI.c and si2dr_liberty.h; and fixed 
       a whooper at the end of si2dr_liberty.h, which would blow up a C++ compiler.
 
2. updated the configure script, to make sure bison is at least at 1.28, and
   gperf at 2.7; if you have been using a bison version older than 22 but younger than
   28, let me know, and I'll set the bar down a few notches. We don't have time
   to try every version; we know 1.28 works, and 1.22 didn't. The M4 macros for the
   bison and gperf checks will be included in the release; copy them to $(prefix)/share/aclocal.

3. Set -Wall on PI.c and got rid of most of the warnings.

4. added frag* to the list that should go on the tar file distribution.
   Funny we hadn't spotted that earlier! Thanks to a certain Motorola engineer for
   this one!

5. put ./ in front of local commands in the Makefile.am.  Thanks to the same sharp 
   Motorola engineer for this one also! He warned that expecting '.' could present
   security issues.

6. Fixed a small bug in PI.c that caused a crash during deletion of nodes.

7. Group, define, and attribute names were being treated in a case-insensitive manner.
   This has been fixed.

8. Scaled_cell groups were being treated as if they defined a name, now they are treated
   as if they reference a name.

9. strings like e0 and e1 were being treated as if number (sci notation). Now, you have to 
   have a number in front of such to be recognized as a number (eg. 1e1, etc).

10. A small memory leak was patched; I doubt whether any have had much leaked via this
    path; there are others that will be attacked in future releases.



Version 1.5:

1. A request was made to allow the parser to allow a semicolon after a group's closing 
   curly. This was done, a little test added to the suite to make sure this feature
   isn't lost over time.


Version 1.4:

1. Modified the lexer to enforce the rule that : be preceded by whitespace (space or tab). 
   It will issue an error message, but procede as normal.

2. Libtool base upgraded to libtool 1.4.2


Version 1.3:

1. Added the include_file capability to the parser. It doesn't have access
   to the search path in the library compiler, so as a compromise, if
   you set the environment variable LIBERTY_PARSER_SEARCH_PATH to the
   colon separated list of directories to search, it will check these
   directories if the file cannot be found in the current directory.
   I guess this would be a loser on Windows....

   The format of all the error messages was upgraded to include the file 
   name. This probably will mess up all your regression tests. It did it 
   to most of mine. They are all updated now with new ref files.

   The PI was upgraded to include the si2drObjectGetFileName and 
   si2drObjectSetFileName functions. The parser uses these functions to
   set the file name and line numbers on each group, attribute and define
   node.

2. I added code to generate error messages if a \ at the end of a line
   is followed by any spaces or tabs. The library compiler will do the 
   same.

3. I added code to detect in the timing group, if cell_fall is specified,
   then transition_fall should also be; and the corresponding for cell_rise. 


Version 1.2:

1. If you had a values(0), instead of values("0"), the program would 
   generate errors. This is because it things values(0) is a bus name. 
   I fixed this for values; I had to play around with the lexer to do this. 

2. I straightened out a few error messages to make them more visible
   and more understandable.

3. I added the ability to read compressed files directly: if a file name
   ends with .gz, .bz2 or .Z, the proper compression program is called
   (if it is in your PATH) as an input filter.

4. If there was a "VDD expression" in the .lib file, and VDD is not
   the first term, then there would be an error. I made it so VDD can be
   either the beginning or ending term. I also made it so the operations
   are not only just '-' and '+', but also '*' and '/' now.


Version 1.1:

1. Added code to process define_group statements. This included
   parser and lexer.

2. Modified the checking code to take the define_groups into account.

