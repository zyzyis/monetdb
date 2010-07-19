import os, time, sys
from MonetDBtesting import process

def server_start(args):
    sys.stderr.write('#mserver: "%s"\n' % ' '.join(args))
    sys.stderr.flush()
    srv = process.server('sql', args = args, stdin = process.PIPE,
                         stdout = process.PIPE, stderr = process.PIPE)
    time.sleep(5)                      # give server time to start
    return srv

def client(lang, file):
    sys.stderr.write('#client: "%s"\n' % file)
    sys.stderr.flush()
    clt = process.client(lang.lower(), stdin = open(file),
                         stdout = process.PIPE, stderr = process.PIPE)
    return clt.communicate()

def main():
    srv = server_start(["--set", "sql_debug=64"])
    out, err = client('SQL',
                      os.path.join(os.getenv('RELSRCDIR'),
                                   'mdb_starts_with_sql_debug_64.SF-1999354.sql'))
    sys.stdout.write(out)
    sys.stderr.write(err)
    out, err = srv.communicate()
    sys.stdout.write(out)
    sys.stderr.write(err)

main()
