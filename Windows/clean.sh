#!/bin/sh

echo "rm -rf debug release tlsh.VC2005.ncb tlsh.VC2005.suo"
      rm -rf debug release tlsh.VC2005.ncb tlsh.VC2005.suo

echo "rm -rf tlsh_dll/Debug tlsh_dll/Release tlsh_dll/tlsh.VC2005.vcproj.*"
      rm -rf tlsh_dll/Debug tlsh_dll/Release tlsh_dll/tlsh.VC2005.vcproj.*

echo "rm -rf tlsh_unittest/Debug tlsh_unittest/Release tlsh_unittest/tlsh_unittest.VC2005.vcproj.*"
      rm -rf tlsh_unittest/Debug tlsh_unittest/Release tlsh_unittest/tlsh_unittest.VC2005.vcproj.*

echo "rm -rf tlsh_version/Debug tlsh_version/Release tlsh_version/tlsh_version.VC2005.vcproj.*"
      rm -rf tlsh_version/Debug tlsh_version/Release tlsh_version/tlsh_version.VC2005.vcproj.*
