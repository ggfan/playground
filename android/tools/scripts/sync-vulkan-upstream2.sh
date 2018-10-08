#!/bin/bash
# Functionality:
# 1) pull down the repo from github for vulkan_basic_samples and lunarG/VulkanSamples
# 2) merge upstream into downstream
#        but keeps README.md
#        remove .appveyor.yml .travis.yml
#
if [ ! ["${OSTYPE}"=="linux"*] ]; then
	exit $?
fi

# put --amend or leave it to nothing
COMMIT_CMD_METHOD=

WORKSPACE=${HOME}/tmp/workspace
rm -fr ${WORKSPACE}

mkdir  ${WORKSPACE}
pushd  ${WORKSPACE}

#clone the 2 repo to src and dst sub directory
# git clone git@github.com:LunarG/VulkanSamples.git src
git clone git@github.com:googlesamples/vulkan-basic-samples.git dst

mkdir  ./tmp

# save the files we want to preserve
cp -f dst/README.md  tmp/

pushd dst
git fetch origin

git remote add upstream git@github.com:LunarG/VulkanSamples.git
git fetch upstream
git checkout -b upstream-master  upstream/master
git pull upstream master

git checkout -b merge-upstream  origin/master
git reset --hard
git pull origin master

git merge -X theirs upstream-master
rm -f .appveyer.yml  .travis.yml

# regenerate all project files
pushd  API-Samples
cmake -DANDROID=ON -DABI_NAME=armeabi-v7a
rm -fr CMakeFiles Makefile CMakeCache.txt
popd

gradle_files=(settings.gradle build.gradle gradle.properties gradlew gradlew.bat gradle)
for f in ${gradle_files[*]}; do
	rm -fr ./${f}
  mv -f API-Samples/android/${f}  ./
done

sed  -ie "s/':/'API-Samples\/android\/:/g"  ./settings.gradle
popd

# restore preserved files
mv -f  tmp/README.md   dst/

# commit changes
pushd dst
git add --all
git commit --amend --no-edit

#git push -f origin merge-upstream:merge-upstream
popd

# clean up
popd
#rm -fr ${WORKSPACE}
