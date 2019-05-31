#!/bin/sh

echo "rm -rf debug release tlsh.VC*.ncb tlsh.VC*.suo"
      rm -rf debug release tlsh.VC*.ncb tlsh.VC*.suo

echo "rm -rf tlsh_dll/Debug tlsh_dll/Release tlsh_dll/tlsh.VC*.vcproj.*"
      rm -rf tlsh_dll/Debug tlsh_dll/Release tlsh_dll/tlsh.VC*.vcproj.*

echo "rm -rf tlsh_unittest/Debug tlsh_unittest/Release tlsh_unittest/tlsh_unittest.VC*.vcproj.*"
      rm -rf tlsh_unittest/Debug tlsh_unittest/Release tlsh_unittest/tlsh_unittest.VC*.vcproj.*

echo "rm -rf tlsh_version/Debug tlsh_version/Release tlsh_version/tlsh_version.VC*.vcproj.*"
      rm -rf tlsh_version/Debug tlsh_version/Release tlsh_version/tlsh_version.VC*.vcproj.*

echo "rm -rf simple_unittest/Debug simple_unittest/Release simple_unittest/simple_unittest.vcproj.*"
      rm -rf simple_unittest/Debug simple_unittest/Release simple_unittest/simple_unittest.vcproj.*

echo "rm -f ../bin/*.dll ../bin/*.exe"
      rm -rf ../bin/*.dll ../bin/*.exe
