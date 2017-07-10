#!/bin/bash
if [ ! ["${OSTYPE}"=="linux"*] ]; then
	exit $?
fi

WORKSPACE=${HOME}/tmp/workspace
rm -fr ${WORKSPACE}

mkdir  ${WORKSPACE}
pushd  ${WORKSPACE}

#clone the 2 repo to src and dst sub directory
git clone git@github.com:LunarG/VulkanSamples.git src
git clone git@github.com:googlesamples/vulkan-basic-samples.git dst
mkdir  ./tmp

pushd  src/API-Samples
cmake -DANDROID=ON -DANDROID_ABI=all
rm -fr CMakeFiles Makefile CMakeCache.txt
popd

# save the files we want to preserve
mv -f dst/README.md  tmp/
mv -f dst/settings.gradle  tmp/

# preserve the src's git history
mv -f src/.git  tmp/src/
mv -f src/.gitignore tmp/src/



cp -fr src/*   dst/
gradle_files=(settings.gradle build.gradle gradle.properties gradlew gradlew.bat gradle)
for f in ${gradle_files[*]}; do
	rm -fr dst/${f}
    mv -f dst/API-Samples/android/${f}  dst/
done

sed  -ie "s/':/'API-Samples\/android\/:/g"  dst/settings.gradle

# restore preserved files
mv -f  tmp/README.md   dst/
#mv -f  tmp/.gitignore  dst/


# clean up
popd
#rm -fr ${WORKSPACE}
