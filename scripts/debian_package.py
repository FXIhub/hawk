#!/usr/bin/env python

import os
import sys
import datetime
import subprocess

package = "hawk"

if(not os.path.exists("/etc/debian_version")):
	print "Cannot find /etc/debian_version!"
	print "You must run this script on a Debian machine."
	sys.exit(1)

subprocess.call("sudo apt-get install -y build-essential devscripts debhelper",shell=True)
#cwd = os.getcwd()
#if(not cwd.endswith("scripts")):
#   print "You need to be inside the \"libspimage/scripts\" directory to run the script."
#	sys.exit(2)

date_vers = datetime.date.today().strftime("%Y.%m.%d")
dirname = "/tmp/"+package+"-"+date_vers
fname = "/tmp/"+package+"_"+date_vers+".orig.tar.gz"
dirbase = package+"-"+date_vers
if(not os.path.exists(fname)):
	subprocess.call("wget 'https://github.com/FilipeMaia/"+package+"/tarball/master' -O "+fname,shell=True);
	os.mkdir(dirname)
	os.chdir(dirname)
	subprocess.call("tar --strip-components 1 -zxvf "+fname,shell=True)
	os.chdir("..")
	subprocess.call("tar -zcvf "+fname+" "+dirbase,shell=True)

os.chdir(dirname)
if(not os.path.exists("debian")):
	os.mkdir("debian")
if(not os.path.exists("debian/changelog")):
	subprocess.call(["dch","--create","-v",date_vers+"-1","--package",package])
subprocess.call("echo 8 > debian/compat",shell=True)
fp = open("debian/control","w")
control_string = """Source: """+package+"""
Maintainer: Filipe Maia <filipe.maia@icm.uu.se>
Section: misc
Priority: optional
Standards-Version: 3.9.2
Build-Depends: debhelper (>= 8)

Package: """+package+"""
Architecture: any
Depends: ${shlibs:Depends}, ${misc:Depends}
Description: X-ray diffraction imaging library
 Implements several algorithms used in X-ray diffraction imaging.
"""

fp.write(control_string)
fp.close()
subprocess.call("cp Copyright debian/copyright",shell=True)

rules_string = """#!/usr/bin/make -f
%:
	dh $@
"""
fp = open("debian/rules","w")
fp.write(rules_string)
fp.close()


try:
	os.mkdir("debian/source")
except OSError:
	pass


subprocess.call("echo 3.0 \(quilt\) > debian/source/format",shell=True)

subprocess.call("debuild -us -uc",shell=True);
