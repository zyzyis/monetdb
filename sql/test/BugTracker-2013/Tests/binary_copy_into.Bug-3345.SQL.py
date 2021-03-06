import os, sys, shutil
try:
    from MonetDBtesting import process
except ImportError:
    import process

src = os.environ['RELSRCDIR']
dst = os.environ['TSTTRGDIR']

clt = process.client('sql',
                     stdin = process.PIPE,
                     stdout = process.PIPE,
                     stderr = process.PIPE,
                     interactive = False,
                     echo = False)

shutil.copyfile(os.path.join(src,'n_nationkey.sorted'), os.path.join(dst,'n_nationkey.sorted'))
shutil.copyfile(os.path.join(src,'n_regionkey.sorted'), os.path.join(dst,'n_regionkey.sorted'))

clt.stdin.write('start transaction;');
clt.stdin.write('create table bug (n_nationkey INTEGER,n_regionkey INTEGER);\n')
clt.stdin.write('copy binary into bug from \'%s/n_nationkey.sorted\', \'%s/n_regionkey.sorted\';\n'% (dst,dst))

out, err = clt.communicate()
sys.stdout.write(out.replace(os.environ['TSTTRGBASE'],'${TSTTRGBASE}').replace('\\','/'))
sys.stderr.write(err.replace(os.environ['TSTTRGBASE'],'${TSTTRGBASE}').replace('\\','/'))
