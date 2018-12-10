import os, time, sys, argparse, logging, atexit, signal, json, subprocess, select

##########################################################################

rootLog = logging.getLogger()
info = rootLog.info
debug = rootLog.debug
module = "rsync_sst_"

rsync_pid = None
rsync_pid_file = None
rsync_conf = None

if os.name=="nt":
	mytime = time.clock
else:
	mytime = time.time
	
def check_proc(pid):
	if not pid:
		return False
		
	try:
		os.kill(int(pid), 0)
		return True
	except OSError:
		return False

def remove_file(name):
	if name and os.path.exists(name):
		os.remove(name)
		
def cleanup():
	debug("cleanup")
	if rsync_pid and rsync_pid>0 and check_proc(rsync_pid):
		os.kill(rsync_pid, 9)
	remove_file(rsync_pid_file)
	remove_file(rsync_conf)
	debug("cleanup done")

def sig_cleanup(signum, frame):		
	cleanup()

def die (code, msg):
	rootLog.error(msg)
	sys.exit(code)

def load_sst_state (name):
	sst_state = None
	if not os.path.exists(name):
		return None, "can not read state file %s" % name
		
	with open(name, 'r') as sst_state_file:
		sst_state = json.load(sst_state_file)

	if not sst_state:
		return None, "can not parse state file %s" % name
		
	if "gtid" not in sst_state:
		return None, "no gtid provided"
		
	if "bypass" not in sst_state:
		return None, "no bypass set"

	if "indexes" not in sst_state or (sst_state["bypass"]==False and len(sst_state["indexes"])==0):
		return None, "no indexes provided"
		
	return sst_state, None

def check_io(proc):
	log_level = {proc.stdout: logging.DEBUG, proc.stderr: logging.ERROR}
	ready_to_read = select.select([proc.stdout, proc.stderr], [], [], 1)[0]
	for io in ready_to_read:
		line = io.readline().strip()
		if len(line):
			rootLog.log(log_level[io], line[:-1])		
	
def parse_listen(listen):
	port_sep = listen.find(':')
	addr = listen[:port_sep]
	port = listen[port_sep+1:]
	return (addr, port)
	
class Cmd:
	def __init__(self, cmd):
		self.cmd = cmd
		debug(cmd)

	def run(self):
		try:
			proc = subprocess.Popen(self.cmd, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			while self.wait() and proc.poll() is None:
				check_io(proc)
				debug("waiting rsync complete")
				
		except OSError as e:
			die(32, "rsync failed, %s" % (e.strerror))
		except subprocess.CalledProcessError as e:
			die(32, "rsync failed, %s, %s" % (e.strerror, e.output))
		except Exception as e:
			die(32, "rsync failed, %s, %s" % (type(e), e))
			
		return (proc.returncode, proc.pid)
		
	def run_retry(self, codes):
		for count in xrange(3):
			exit_code, pid = self.run()
			if exit_code is not None and exit_code in codes:
				debug("retry %d on exit_code %d" %(count, exit_code))
				time.sleep(5)
				continue
			else:
				break
				
		return (exit_code, pid)

			
class Donor(Cmd):
	def __init__(self, cmd, parent):
		Cmd.__init__(self, cmd)
		self.parent = parent
		
	def wait(self):
		return check_proc(self.parent) # waiting sync done and parent alive

class Joiner(Cmd):
	def __init__(self, cmd, parent, state_file):
		Cmd.__init__(self, cmd)
		self.parent = parent
		self.state_file = state_file
		
	def wait(self):
		return check_proc(self.parent) and not os.path.exists(self.state_file)
		
##########################################################################	
	
parser = argparse.ArgumentParser()
parser.add_argument("--role", required=True)
parser.add_argument("--datadir", required=True)
parser.add_argument("--parent", required=True)
parser.add_argument("--cluster-name", required=True)
parser.add_argument("--state-in", required=True)
parser.add_argument("--state-out")
parser.add_argument("--address")
parser.add_argument("--listen")
parser.add_argument("--log-file")
args = parser.parse_args()
	
logFormatter = logging.Formatter(fmt="[%(asctime)s] [%(levelname)-5.5s] RPL-SST: %(message)s", datefmt='%a %b %d %H:%M:%S %Y')

if args.log_file:
	fileHandler = logging.FileHandler(args.log_file, mode='a')
	fileHandler.setFormatter(logFormatter)
	fileHandler.setLevel(logging.DEBUG)
	rootLog.addHandler(fileHandler)

consoleHandler = logging.StreamHandler()
consoleHandler.setFormatter(logFormatter)
consoleHandler.setLevel(logging.INFO)
rootLog.addHandler(consoleHandler)

rootLog.setLevel(logging.DEBUG)

# setup
info("sst starting, role %s, cluster %s" % (args.role, args.cluster_name))
module = module + args.cluster_name

atexit.register(cleanup)
signal.signal(signal.SIGTERM, sig_cleanup)
signal.signal(signal.SIGINT, sig_cleanup)	

if args.role=='donor':
	debug("%s, address %s" % (args.role, args.address))
	
	if not args.address:
		die(1, "no address provided")
	addr, port = parse_listen(args.address)	
	
	sst_state, msg = load_sst_state(args.state_in)
	if not sst_state:
		die(1, msg)
		
	info("donor starting rsync of %d indexes(bypass=%d) to '%s' for '%s'..." %(len(sst_state["indexes"]), int(sst_state["bypass"]), sst_state["gtid"], args.address))
	
	indexes = 0
	for idx in sst_state["indexes"].values():
		idx_files = idx["path"] + ".meta"

		rsync_proc = Donor(["rsync", "-v", "--owner", "--group", "--perms", "--ignore-times", "--inplace", "--port", port, idx_files, "%s::%s" % (addr, module)], args.parent)
		# rsync might failed first time due to donor started faster then joiner
		if indexes==0:
			res, rsync_pid = rsync_proc.run_retry([10])
		else:
			res, rsync_pid = rsync_proc.run()
		
		if check_proc(rsync_pid): # rsync daemon alive and parent dead - abort
			break
		# no exception with exit code for background process
		if not check_proc(rsync_pid) and res!=0:
			rootLog.error("rsync failed, exit code %d" % res)
			break
		indexes = indexes + 1
			
	if check_proc(rsync_pid):
		die(32, "rsync alive %d, force exit" % rsync_pid)
		
	if not check_proc(args.parent):
		die(32, "parent terminated %s, force exit" % args.parent)

	if indexes!=len(sst_state["indexes"]):
		die(32, "sync only %d indexes of %d" % (indexes, len(sst_state["indexes"])))
		
	info("donor sending state file '%s'" % args.state_in)

	rsync_proc = Donor(["rsync", "-v", "--owner", "--group", "--perms", "--ignore-times", "--inplace", "--port", port, args.state_in, "%s::%s/%s" % (addr, module, args.state_out)], args.parent)
	res, rsync_pid = rsync_proc.run()
	
	if not check_proc(rsync_pid) and res!=0:
		die(32, "rsync error %d" % res)
	
	if check_proc(rsync_pid):
		die(32, "rsync alive %d, force exit" % rsync_pid)
		
	if not check_proc(args.parent):
		die(32, "parent terminated %s, force exit" % args.parent)

	info("donor finished ok")

elif args.role=='joiner':
	debug("%s, listen %s" % (args.role, args.listen))
	
	if not args.listen:
		die(1, "no address provided")
	addr, port = parse_listen(args.listen)	
	
	rsync_pid_file = os.path.join(os.path.normpath(args.datadir), module + '.pid')
	if os.path.exists(rsync_pid_file):
		die(114, "rsync daemon already running %s" % rsync_pid_file) # EALREADY
		
	rsync_conf = os.path.join(os.path.normpath(args.datadir), module + '.conf')
	with open(rsync_conf, 'w') as rsync_conf_file:
		rsync_conf_file.write("pid file = %s\n" % rsync_pid_file)
		rsync_conf_file.write("use chroot = no\n")
		rsync_conf_file.write("read only = no\n")
		rsync_conf_file.write("timeout = 300\n")
		rsync_conf_file.write("[%s]\n" % module)
		rsync_conf_file.write("path = %s\n" % args.datadir)
		# need explisitly set root user
		if os.geteuid()==0:
			rsync_conf_file.write("uid = root\n")

	info("joiner waiting for data at '%s' through rsync..." % args.datadir)
	
	remove_file(args.state_in)
	
	rsync_proc = Joiner(["rsync", "--daemon", "--no-detach", "--address", addr, "--port", port, "--config", rsync_conf], args.parent, args.state_in)
	res, rsync_pid = rsync_proc.run()
		
	if not check_proc(rsync_pid) and res!=0:
		die(32, "rsync error %d" % res)
	
	if not check_proc(args.parent):
		die(32, "parent terminated %s, force exit" % args.parent)

	if not os.path.exists(args.state_in):
		die(32, "can not find state file %s" % args.state_in)

	sst_state, msg = load_sst_state(args.state_in)
	if not sst_state:
		die(1, msg)

	for idx in sst_state["indexes"].values():
		name = os.path.basename(idx["path"])
		idx["path"] = os.path.join(os.path.normpath(args.datadir), name)

	with open(args.state_in, 'w') as sst_state_file:
		json.dump(sst_state, sst_state_file)
		
	info("joiner finished ok, got %d indexes(bypass=%d), gtid '%s'", len(sst_state["indexes"]), int(sst_state["bypass"]), sst_state["gtid"])
else:
	die(22,"unrecognized role %s" % args.role)
