if test "$1" = "-xlen"
then
    XLEN="xlen"
else
    XLEN="len"
fi  

TMP='tmp'
HASH=`../bin/tlsh_version | head -1 | cut -f1`
CHKSUM=`../bin/tlsh_version | tail -1 | cut -f1`

sh -cx "../bin/tlsh_unittest -r ../Testing/example_data > $TMP/example_data.out"
if test $XLEN = "xlen"
then
    sh -cx "../bin/tlsh_unittest -xlen -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores"
else
    sh -cx "../bin/tlsh_unittest -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores"
fi
if test $XLEN = "xlen"
then
    sh -cx "../bin/tlsh_unittest -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
else
    sh -cx "../bin/tlsh_unittest -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
fi

mv -v $TMP/example_data.out exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP
mv -v $TMP/example_data.scores exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP
mv -v $TMP/example_data.scores.2 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2_EXP
