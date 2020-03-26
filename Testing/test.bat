@echo off
Setlocal EnableDelayedExpansion

SET TLSH_PROG=tlsh.exe
SET SIMP_PROG=simple_unittest.exe
echo Scenario: tlsh	(c++ standard version)...

if EXIST ..\bin\%TLSH_PROG% ( echo found %TLSH_PROG% ) else ( echo error: 128, you must compile %TLSH_PROG% & EXIT /B 127)
if EXIST ..\bin\%SIMP_PROG% ( echo found %SIMP_PROG% ) else ( echo error: 128, you must compile %SIMP_PROG% & EXIT /B 127)
if EXIST tmp ( echo found tmp) else ( echo mkdir tmp & mkdir tmp)

SET HASH=128
SET CHKSUM=1
SET SLDWIN=5
SET VERBOSE=0

SET XLEN="len"
CALL :runit
if NOT !errorlevel! == 0 ( EXIT /B !errorlevel! )

REM SET XLEN="xlen"
REM CALL :runit
REM if NOT !errorlevel! == 0 ( EXIT /B !errorlevel! )

CALL :test7_to_10
DEL out.res
if NOT !errorlevel! == 0 ( EXIT /B !errorlevel! )


EXIT /B 0

REM ############################
REM # THE runit FUNCTION
REM ############################
REM #
REM # this function will be run twice, "" and "-xlen"
REM #
:runit
	if "%XLEN%" == "xlen" ( SET XLEN=xlen& echo & echo "Scenario: not considering len, ...") else ( SET XLEN=len& echo "Scenario: considering len, ...")

	REM ########################################################
	REM # Test 1
	REM #	get the TLSH values for a directory of files
	REM ########################################################

	echo "test 1"

	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -r example_data -o tmp\example_data.out 2> tmp\example_data.err"
				..\bin\%TLSH_PROG% -r example_data -o tmp\example_data.out 2> tmp\example_data.err

	SET EXPECTED_OUT=exp\example_data.%HASH%.%CHKSUM%.%XLEN%.out_EXP
	SET EXPECTED_ERR=exp\example_data.%HASH%.%CHKSUM%.%XLEN%.err_EXP
	if NOT EXIST %EXPECTED_OUT% ( echo "error: 1, Expected Result file %EXPECTED_OUT% does not exist" & EXIT /B 1)
	if NOT EXIST %EXPECTED_ERR% ( echo "error: 1, Expected Result file %EXPECTED_ERR% does not exist" & EXIT /B 1)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.out %EXPECTED_OUT%
			       fc /W tmp\example_data.out %EXPECTED_OUT% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (1), fc tmp/example_data.out %EXPECTED_OUT%" & EXIT /B 1)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.out %EXPECTED_ERR%
			       fc /W tmp\example_data.err %EXPECTED_ERR% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (1), fc tmp\example_data.out %EXPECTED_ERR%" & EXIT /B 1)

	echo "passed"

	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -r example_data -o tmp\example_data.json_out -ojson 2> tmp\example_data.err"
				..\bin\%TLSH_PROG% -r example_data -o tmp\example_data.json_out -ojson 2> tmp\example_data.err
	SET EXPECTED_OUT=exp\example_data.dos_%XLEN%.json_out_EXP
	if NOT EXIST %EXPECTED_OUT% ( echo "error: 1, Expected Result file %EXPECTED_OUT% does not exist" & EXIT /B 1)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.json_out %EXPECTED_OUT%
			       fc /W tmp\example_data.json_out %EXPECTED_OUT% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (1), fc tmp\example_data.json_out %EXPECTED_OUT%" & EXIT /B 1)

	echo "passed"

	REM ########################################################
	REM # Test 2
	REM #	calculate scores of a file (website_course_descriptors06-07.txt) compared to the directory of files
	REM ########################################################

	echo "test 2"

	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -r example_data -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores 2> tmp\example_data.err2"
				..\bin\%TLSH_PROG% -r example_data -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores 2> tmp\example_data.err2

	SET EXPECTED_SCO=exp\example_data.%HASH%.%CHKSUM%.%SLDWIN%.%XLEN%.dos_scores_EXP
	SET EXPECTED_ERR=exp\example_data.%HASH%.%CHKSUM%.%XLEN%.err2_EXP
	if NOT EXIST %EXPECTED_SCO% ( echo "error: 1, Expected Result file %EXPECTED_SCO% does not exist" & EXIT /B 1)
	if NOT EXIST %EXPECTED_ERR% ( echo "error: 1, Expected Result file %EXPECTED_ERR% does not exist" & EXIT /B 1)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.scores %EXPECTED_SCO%
			       fc /W tmp\example_data.scores %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (2), fc tmp/example_data.scores %EXPECTED_SCO%" & EXIT /B 2)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.err2 %EXPECTED_ERR%
			       fc /W tmp\example_data.err2 %EXPECTED_ERR% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (2), fc tmp/example_data.err2 %EXPECTED_ERR%" & EXIT /B 2)

	echo "passed"

	REM ########################################################
	REM # Test 3
	REM #	calculate scores of a file (website_course_descriptors06-07.txt) compared to hashes listed in a file
	REM #	far more efficient
	REM ########################################################

	echo "test 3"

	REM # note that test 3 will output the following error, so write stderr to /dev/null, so it will not be seen.
	REM #   warning: cannot read TLSH code ../Testing/example_data/BookingBrochure.txt
	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -l tmp\example_data.out -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.2"
				..\bin\%TLSH_PROG% -l tmp\example_data.out -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.2 2>NUL

	SET EXPECTED_SCO=exp\example_data.%HASH%.%CHKSUM%.%SLDWIN%.%XLEN%.dos_scores.2_EXP

	if %VERBOSE% == 1 echo fc /W tmp\example_data.scores.2 %EXPECTED_SCO%
			       fc /W tmp\example_data.scores.2 %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (3), fc tmp/example_data.scores.2 %EXPECTED_SCO%" & EXIT /B 3)

	echo "passed"


	REM ########################################################
	REM # Test 4
	REM #	Test out the -xref parameter which computes the distance scores for each file in a directory (-r parameter) with 
	REM #   all other files in that directory.
	REM ########################################################
	SET testnum=4
	echo "test %testnum%"
	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -xref -r example_data -o tmp\example_data.xref.scores"
				..\bin\%TLSH_PROG% -xref -r example_data -o tmp\example_data.xref.scores 2>NUL

	SET EXPECTED_SCO=exp/example_data.%HASH%.%CHKSUM%.%XLEN%.xref.scores_EXP

	if %VERBOSE% == 1 echo fc /W tmp\example_data.xref.scores %EXPECTED_SCO%
			       fc /W tmp\example_data.xref.scores %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (4), fc /W tmp\example_data.xref.scores %EXPECTED_SCO%" & EXIT /B 4)

	echo "passed"

	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -xref -r example_data -o tmp\example_data.xref.json_scores -ojson"
				..\bin\%TLSH_PROG% -xref -r example_data -o tmp\example_data.xref.json_scores -ojson 2>NUL
	SET EXPECTED_SCO=exp/example_data.dos_%XLEN%.xref.json_scores_EXP
	if %VERBOSE% == 1 echo fc /W tmp\example_data.xref.json_scores %EXPECTED_SCO%
			       fc /W tmp\example_data.xref.json_scores %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (4), fc /W tmp\example_data.xref.json_scores %EXPECTED_SCO%" & EXIT /B 4)

	echo "passed"

	REM ########################################################
	REM # Test 5
	REM #	Test out the -T (threshold parameter)
	REM ########################################################
	SET testnum=5
	echo "test 5"
	REM # note that test 5 will output the following error, so write stderr to /dev/null, so it will not be seen.
	REM #   warning: cannot read TLSH code ../Testing/example_data/BookingBrochure.txt

	if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -T 201 -l tmp\example_data.out -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.2.T-201"
				..\bin\%TLSH_PROG% -T 201 -l tmp\example_data.out -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.2.T-201 2>NUL

	SET EXPECTED_SCO=exp/example_data.%HASH%.%CHKSUM%.%SLDWIN%.%XLEN%.dos_scores.2.T-201_EXP
	if NOT EXIST %EXPECTED_SCO% ( echo "error: 1, Expected Result file %EXPECTED_SCO% does not exist" & EXIT /B 5)

	if %VERBOSE% == 1 echo fc /W tmp\example_data.scores.2.T-201 %EXPECTED_SCO%
			       fc /W tmp\example_data.scores.2.T-201 %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
	set /p diffc=<out.res
	if "%diffc%" == "Different" ( echo "error: (5), fc tmp/example_data.scores.2.T-201 %EXPECTED_SCO%" & EXIT /B 5)

	echo "passed"
EXIT /B 0

REM ############################
REM # END OF THE runit FUNCTION
REM ############################

REM ############################
REM # Test 6
REM #	Test out the TLSH digest with a wide range of lengths (testlen.sh)
REM need to rewrite testlen.sh as testlen.bat
REM ############################
REM 
REM # I use the papameter value of 22 for the Fibanacci sequence for generating content
REM # this generates files up to 6.7 Meg (good enough for automated testing)
REM 
REM echo "./testlen.sh $TLSH_PROG 22 > tmp/testlen.out"
REM       ./testlen.sh $TLSH_PROG 22 > tmp/testlen.out
REM 
REM EXPECTED_TESTLEN=exp/testlen.%HASH%.%CHKSUM%.%SLDWIN%.out_EXP
REM if test ! -f $EXPECTED_TESTLEN
REM (
REM 		echo "error: ($testnum), Expected Result file $EXPECTED_TESTLEN does not exist"
REM 		EXIT /B 1
REM )
REM 
REM diff --ignore-all-space tmp/testlen.out $EXPECTED_TESTLEN > /dev/null 2>/dev/null
REM if [ $? -ne 0 ]; (
REM 	echo "error: ($testnum) fc tmp/testlen.out $EXPECTED_TESTLEN"
REM 	EXIT /B $testnum
REM )
REM echo "passed"
REM 
REM ############################
REM # END OF test 6
REM ############################
REM 
REM ############################
REM # Test 7
REM #	Test the -force option
REM ############################

:test7_to_10

echo "test 7"

if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -force -f example_data\small.txt -o tmp\small.tlsh"
			..\bin\%TLSH_PROG% -force -f example_data\small.txt -o tmp\small.tlsh

SET EXPECTED_TLSH=exp/small.%HASH%.%CHKSUM%.%SLDWIN%.dos_tlsh_EXP
if NOT EXIST %EXPECTED_TLSH% ( echo "error: 1, Expected Result file %EXPECTED_TLSH% does not exist" & EXIT /B 7)

if %VERBOSE% == 1 echo fc /W tmp\small.tlsh %EXPECTED_TLSH%
		       fc /W tmp\small.tlsh %EXPECTED_TLSH% >NUL && Echo Same>out.res || Echo Different>out.res
set /p diffc=<out.res
if "%diffc%" == "Different" ( echo "error: (7), fc tmp/small.tlsh %EXPECTED_TLSH%" & EXIT /B 7)



if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -force -f example_data\small2.txt -o tmp\small2.tlsh"
			..\bin\%TLSH_PROG% -force -f example_data\small2.txt -o tmp\small2.tlsh

SET EXPECTED_TLSH=exp/small2.%HASH%.%CHKSUM%.%SLDWIN%.dos_tlsh_EXP
if NOT EXIST %EXPECTED_TLSH% ( echo "error: 1, Expected Result file %EXPECTED_TLSH% does not exist" & EXIT /B 7)

if %VERBOSE% == 1 echo fc /W tmp\small2.tlsh %EXPECTED_TLSH%
		       fc /W tmp\small2.tlsh %EXPECTED_TLSH% >NUL && Echo Same>out.res || Echo Different>out.res
set /p diffc=<out.res
if "%diffc%" == "Different" ( echo "error: (7), fc tmp/small2.tlsh %EXPECTED_TLSH%" & EXIT /B 7)

echo "passed"

REM ############################
REM # END OF test 7
REM ############################
REM 
REM ############################
REM # Test 8
REM #	Test the -l2 and -lcsv options
REM ############################

echo "test 8"

REM #
REM # Test 8(a): -l2
REM #
if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -T 201 -l2 -l example_data_col_swap.tlsh -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.l2.T-201"
			..\bin\%TLSH_PROG% -T 201 -l2 -l example_data_col_swap.tlsh -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.l2.T-201 2>NUL

REM # same expected output as Test 5

SET EXPECTED_SCO=exp/example_data.%HASH%.%CHKSUM%.%SLDWIN%.len.dos_scores.2.T-201_EXP
if NOT EXIST %EXPECTED_SCO% ( echo "error: 1, Expected Result file %EXPECTED_SCO% does not exist" & EXIT /B 8)

if %VERBOSE% == 1 echo fc /W tmp\example_data.scores.l2.T-201 %EXPECTED_SCO%
		       fc /W tmp\example_data.scores.l2.T-201 %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
set /p diffc=<out.res
if "%diffc%" == "Different" ( echo "error: (8), fc tmp\example_data.scores.l2.T-201 %EXPECTED_SCO%" & EXIT /B 8)

REM #
REM # Test 8(a): -l2 -lcsv
REM #
if %VERBOSE% == 1 echo "..\bin\%TLSH_PROG% -T 201 -l2 -lcsv -l example_data_col_swap.csv -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.l2csv.T-201"
			..\bin\%TLSH_PROG% -T 201 -l2 -lcsv -l example_data_col_swap.csv -c example_data\website_course_descriptors06-07.txt -o tmp\example_data.scores.l2csv.T-201 2>NUL

REM # same expected output as Test 8(a) above

if %VERBOSE% == 1 echo fc /W tmp\example_data.scores.l2csv.T-201 %EXPECTED_SCO%
		       fc /W tmp\example_data.scores.l2csv.T-201 %EXPECTED_SCO% >NUL && Echo Same>out.res || Echo Different>out.res
set /p diffc=<out.res
if "%diffc%" == "Different" ( echo "error: (8), fc tmp\example_data.scores.l2csv.T-201 %EXPECTED_SCO%" & EXIT /B 8)

echo "passed"


REM ############################
REM # END OF test 8
REM ############################

REM ############################
REM # Test 9
REM #	Test the -split option
REM	-split is not working relaibly on windows - I need to fix this...
REM ############################

REM echo "test 9"

REM echo "..\bin\%TLSH_PROG% -split 50,100,200 -f example_data\Week3.txt -o tmp\example_data.Week3.split.tlsh"
REM       ..\bin\%TLSH_PROG% -split 50,100,200 -f example_data\Week3.txt -o tmp\example_data.Week3.split.tlsh   2>NUL

REM SET EXPECTED_RES=exp/example_data.Week3.split.tlsh
REM if NOT EXIST %EXPECTED_RES% ( echo "error: 1, Expected Result file %EXPECTED_RES% does not exist" & EXIT /B 9)

REM echo fc /W tmp\example_data.Week3.split.tlsh %EXPECTED_RES%
REM      fc /W tmp\example_data.Week3.split.tlsh %EXPECTED_RES% >NUL && Echo Same>out.res || Echo Different>out.res
REM set /p diffc=<out.res
REM if "%diffc%" == "Different" ( echo "error: (8), fc tmp\example_data.Week3.split.tlsh %EXPECTED_RES%" & EXIT /B 9)

REM echo "passed"

REM ############################
REM # END OF test 9
REM ############################

REM ############################
REM # test 10
REM ############################

echo "Running %SIMP_PROG%"
..\bin\%SIMP_PROG% > tmp\simple_unittest.out

SET EXPECTED_STEST=exp\simple_unittest.%HASH%.%CHKSUM%.EXP
if NOT EXIST %EXPECTED_STEST% ( echo "error: 1, Expected Result file %EXPECTED_STEST% does not exist" & EXIT /B 10)

if %VERBOSE% == 1 echo fc /W tmp\simple_unittest.out %EXPECTED_STEST%
		       fc /W tmp\simple_unittest.out %EXPECTED_STEST% >NUL && Echo Same>out.res || Echo Different>out.res
set /p diffc=<out.res
if "%diffc%" == "Different" ( echo "error: (8), fc tmp\simple_unittest.out %EXPECTED_STEST%" & EXIT /B 10)

echo "passed all example data tests"

echo.
echo "If you have made changes to the Tlsh python module, build and install it, and run python_test.sh"
echo.

EXIT /B 0
