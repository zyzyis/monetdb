# The contents of this file are subject to the MonetDB Public
# License Version 1.0 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at
# http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
# 
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
# 
# The Original Code is the Monet Database System.
# 
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-2004 CWI.
# All Rights Reserved.
# 
# Contributor(s):
# 		Martin Kersten <Martin.Kersten@cwi.nl>
# 		Peter Boncz <Peter.Boncz@cwi.nl>
# 		Niels Nes <Niels.Nes@cwi.nl>
# 		Stefan Manegold  <Stefan.Manegold@cwi.nl>

import sys, os

if len(sys.argv) != 3:
    print '''\
Usage: %s srcdir dstdir

where srcdir is the top of the MonetDB source directory tree, and
dstdir is the top of the installation tree.''' % sys.argv[0]
    sys.exit(1)

srcdir = sys.argv[1]
dstdir = sys.argv[2]

# Here a hack to make this work on Windows:
# The command name cannot have any spaces, not even quoted, so we
# can't use (as we used to) os.path.join(dstdir, 'bin', 'Mx') for the
# value of mx.  Instead we add the bin directory to the beginning of
# the command search path.  We assume there is a PATH in the
# environment, but that doesn't seem like to big an assumption.
os.environ['PATH'] = os.path.join(dstdir, 'bin') + os.pathsep + os.environ['PATH']
mx = 'Mx'

# figure out a place for temporary files
import tempfile
tmpdir = tempfile.gettempdir()

def copyfile(srcfile, dstfile):
    print 'copyfile', srcfile, dstfile
    f = open(srcfile, 'rb')
    data = f.read()
    f.close()
    f = open(dstfile, 'wb')
    f.write(data)
    f.close()

def unlink(file):
    try:
        os.unlink(file)
    except os.error:
        pass

def runMx(srcdir, base, dstdir):
    print 'runMx', srcdir, base, dstdir
    cmd = '%s "-R%s" -H1 -w "%s" 2>&1' % (mx, tmpdir, os.path.join(srcdir, base + '.mx'))
    print cmd
    f = os.popen(cmd, 'r')
    dummy = f.read()                    # discard output
    f.close()
    srcfile = os.path.join(tmpdir, base + '.body.html')
    if not os.path.exists(srcfile):
        srcfile = os.path.join(tmpdir, base + '.html')
    copyfile(srcfile, os.path.join(dstdir, base + '.html'))
    unlink(os.path.join(tmpdir, base + '.html'))
    unlink(os.path.join(tmpdir, base + '.index.html'))
    unlink(os.path.join(tmpdir, base + '.body.html'))

def removedir(dir):
    """Remove DIR recursively."""
    print 'removedir',dir
    if not os.path.exists(dir):
        return
    names = os.listdir(dir)
    for name in names:
        fn = os.path.join(dir, name)
        if os.path.isdir(fn):
            removedir(fn)
        else:
            os.remove(fn)
    os.rmdir(dir)

removedir(os.path.join(dstdir, 'doc', 'www'))
os.makedirs(os.path.join(dstdir, 'doc', 'www', 'modules'))

for f in ['monet.gif', 'mel.gif']:
    copyfile(os.path.join(srcdir, 'doc', f),
             os.path.join(dstdir, 'doc', f))
for f in ['bat.gif', 'bat1.gif', 'bat2.gif']:
    copyfile(os.path.join(srcdir, 'src', 'gdk', f),
             os.path.join(dstdir, 'doc', 'www', f))

for f in ['monet', 'mil', 'mel']:
    runMx(os.path.join(srcdir, 'doc'), f, os.path.join(dstdir, 'doc'))

runMx(os.path.join(srcdir, 'src', 'mel'), 'mel',
      os.path.join(dstdir, 'doc', 'www'))

for f in ['gdk', 'gdk_atoms']:
    runMx(os.path.join(srcdir, 'src', 'gdk'), f,
          os.path.join(dstdir, 'doc', 'www'))

runMx(os.path.join(srcdir, 'src', 'monet'), 'monet',
      os.path.join(dstdir, 'doc', 'www'))

for f in ['Mapi', 'MapiClient']:
    runMx(os.path.join(srcdir, 'src', 'mapi', 'clients', 'C'), f,
          os.path.join(dstdir, 'doc', 'www'))

runMx(os.path.join(srcdir, 'src', 'tools'), 'Mserver',
      os.path.join(dstdir, 'doc', 'www'))

runMx(os.path.join(srcdir, 'src', 'mapi'), 'mapi',
      os.path.join(dstdir, 'doc', 'www', 'modules'))

runMx(os.path.join(srcdir, 'src', 'modules', 'calibrator'), 'calib',
      os.path.join(dstdir, 'doc', 'www', 'modules'))

for f in ['aggrX3', 'aggr', 'alarm', 'algebra', 'arith', 'ascii_io', 'bat',
          'blob', 'counters', 'decimal', 'enum', 'kernel',
          'lock', 'mmath', 'monettime', 'pcl', 'radix', 'streams', 'str', 'sys',
          'tcpip', 'trans', 'unix', 'url', 'xtables']:
    runMx(os.path.join(srcdir, 'src', 'modules', 'plain'), f,
          os.path.join(dstdir, 'doc', 'www', 'modules'))

for f in ['bitset', 'bitvector', 'ddbench', 'mel', 'mprof', 'oo7', 'qt', 'tpcd',
          'wisc']:
    runMx(os.path.join(srcdir, 'src', 'modules', 'contrib'), f,
          os.path.join(dstdir, 'doc', 'www', 'modules'))

for f in ['README', 'load.mil', 'init.mil']:
    copyfile(os.path.join(srcdir, 'scripts', 'gold', f),
             os.path.join(dstdir, 'doc', 'www', f))
for f in ['HowToStart', 'HowToStart-Win32.txt']:
    copyfile(os.path.join(srcdir, f),
             os.path.join(dstdir, 'doc', 'www', f))

f = open(os.path.join(dstdir, 'doc', 'www', 'sql.html'), 'w')
f.write('''\
<html>
  <body>
    <h3>
      <a href="mailto:niels@cwi.nl">The SQL frontend is not documented yet</a>
    </h3>
  </body>
</html>
''')
f.close()
