#!/bin/bash

CI_ROOT=/media/gfan/source/nightly
SRC_ROOT=$CI_ROOT/src
BRANCH_NAME=pie-gsi

LOG_FILE=${CI_ROOT}/result/log.txt
RESULT_FILE=${CI_ROOT}/result/result.txt
SESSION_RESULT=${CI_ROOT}/result/tmp_result.txt

declare lunch_menu=(aosp_arm64-userdebug aosp_arm64_ab-userdebug)
declare out_dirs=(generic_arm64 generic_arm64_ab)

pushd $CI_ROOT
rm  -fr $SRC_ROOT

mkdir  -p $SRC_ROOT/$BRANCH_NAME
cd $SRC_ROOT/$BRANCH_NAME
repo init -u https://android.googlesource.com/platform/manifest -b $BRANCH_NAME
repo sync -cq -j8

rm -f $LOG_FILE $SESSION_RESULT
for (( i=0; i<${#lunch_menu[@]}; i++ )); do
	# set up the environment
	source build/envsetup.sh
	this_menu="${lunch_menu[$i]}"
	#lunch ${lunch_menu[$i]}
	lunch $this_menu >>$LOG_FILE

	make clean
	make -j >>$LOG_FILE

	echo "$(echo "====")" "$(date)" ":$this_menu" "$(echo "====")" >>$SESSION_RESULT

	# check for system.img file
	if [ -f out/target/product/${out_dirs[$i]}/system.img ]; then
    	stat -c "%y %s %n" out/target/product/${out_dirs[$i]}/system.img >> $SESSION_RESULT
	else
		echo "    build gsi" "$this_menu" "failed" >> $SESSION_RESULT
	fi

    # build vts
    ## assuming machine is setup correctly according to codelab:
    ## https://codelabs.developers.google.com/codelabs/android-vts/#1
	source $CI_ROOT/bin/setup_vts_py.sh
	make vts -j >> $LOG_FILE

	if [ -f out/host/linux-x86/vts/android-vts.zip ]; then
    	stat -c "%y %s %n" out/host/linux-x86/vts/android-vts.zip >> $SESSION_RESULT
	else
		echo "    build vts" "$this_menu" "failed" >> $SESSION_RESULT
	fi
done

#combine the log file: latest one at the top
TMP_FILE=$CI_ROOT/result/tmp.txt
cat $SESSION_RESULT $RESULT_FILE > $TMP_FILE
cat $TMP_FILE > $RESULT_FILE

#clean up
rm -f $TMP_FILE $SESSION_RESULT

popd
