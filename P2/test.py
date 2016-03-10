import subprocess
import socket
import sys
import threading
import time
import unittest

Cmdline = ["./chatsrv"]
IP = "127.0.0.1"
Port = 3100

def waitFor(func, timeout=0.5):
    t = time.time()

    while (time.time() - t) < timeout:
        if func():
            return True
        time.sleep(0.001)

    return False

class SocketReader(object):
    def __init__(self, socket):
        self.socket = socket

class PipeReader(object):
    def __init__(self, pipe):
        self.pipe = pipe
        self.data = ""
        self.lock = threading.Lock()
        self.worker = threading.Thread(target=self._reader)
        self.worker.start()

    def join(self):
        self.worker.join()

    def flush(self):
        with self.lock:
            self.data = ""

    def countString(self, string, flush=False):
        with self.lock:
            ret = self.data.count(string)
            if flush:
                self.data = ""

        return ret

    def _reader(self):
        while True:
            line = self.pipe.readline()
            if not line:
                break

            sys.stderr.write("> " + line)
            with self.lock:
                self.data += line
        sys.stderr.write("<exiting reader>\n")

class TestBase(unittest.TestCase):
    def setUp(self):
        sys.stderr.write("Staring server.\n")
        self.server = subprocess.Popen(Cmdline, stdout=subprocess.PIPE)
        self.reader = PipeReader(self.server.stdout)
        time.sleep(0.1)

    def tearDown(self):
        sys.stderr.write("Stopping server.\n")
        self.server.kill()
        self.reader.join()

    def newClient(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(1)
        s.connect((IP, Port))
        return s

class Test1(TestBase):
    def test_connect(self):
        try:
            c = self.newClient()
            c.close()
        except:
            self.assertTrue(False, "Failed to connect to the server. Is server accepting connections?");

    def test_printString(self):
        c = self.newClient()
        c.close()
            
        self.assertTrue(waitFor(lambda: self.reader.countString("accepted connection") == 1),
            "Failed to wait for 'accepted connection' string in logs.")

    def test_printString2(self):
        c1 = self.newClient()
        c2 = self.newClient() 
        c2.close()

        self.assertTrue(waitFor(lambda: self.reader.countString("accepted connection") == 2),
            "Failed to wait for 2 'accepted connection' string in logs.")

        time.sleep(0.05)
        self.assertTrue(self.reader.countString("accepted connection") == 2,
            "Invalid number of 'accepted connection' strings in logs.")

        c3 = self.newClient()
        c4 = self.newClient()
        c3.close()
        c4.close()
        c1.close()
        self.assertTrue(waitFor(lambda: self.reader.countString("accepted connection") == 4),
            "Failed to wait for 2 new 'accepted connection' string in logs.")


class Test2(TestBase):
    def test_read1(self):
        c = self.newClient()
        c.sendall("TEST-TEST\n")
        c.close()

        self.assertTrue(waitFor(lambda: self.reader.countString("TEST-TEST") == 1),
                            "Failed to wait for a test string in logs.")

    def test_nosplit(self):
        c = self.newClient()
        c.sendall("TEST-")
        time.sleep(0.05)
        self.assertTrue(self.reader.countString("TEST-") == 0,
                            "Partial message was sent.")

        c.sendall("TEST\n")
        c.close()
        self.assertTrue(waitFor(lambda: self.reader.countString("TEST-TEST") == 1),
                            "Failed to wait for a test string in logs.")

    def test_readMulti(self):
        c = self.newClient()
        c.sendall("TEST-TEST\n")

        c2 = self.newClient()
        c2.sendall("TEST-TEST\n")

        c.sendall("TEST-TEST\n")

        self.assertTrue(waitFor(lambda: self.reader.countString("TEST-TEST") == 3),
                            "Failed to wait for a test string in logs.")

        c.close()
        c2.close()

class Test3(TestBase):
    def test_disconnect(self):
        c = self.newClient()
        time.sleep(0.05)

        self.assertTrue(self.reader.countString("connection terminated") == 0,
            "Invalid count of 'connection terminated' strings in logs.")

        c.close()
            
        self.assertTrue(waitFor(lambda: self.reader.countString("connection terminated") == 1),
            "Invalid count of 'connection terminated' strings in logs.")

    def test_disconnect1(self):
        c = self.newClient()
        c1 = self.newClient()
        c2 = self.newClient()
        c1.sendall("Some message")

        time.sleep(0.05)
        self.assertTrue(self.reader.countString("connection terminated") == 0,
            "Invalid count of 'connection terminated' strings in logs.")

        c.close()
            
        self.assertTrue(waitFor(lambda: self.reader.countString("connection terminated") == 1),
            "Invalid count of 'connection terminated' strings in logs.")
        c1.close()
        c2.close()
        self.assertTrue(waitFor(lambda: self.reader.countString("connection terminated") == 3),
            "Invalid count of 'connection terminated' strings in logs.")

class Test4(TestBase):
    def test_greeting(self):
        c1 = self.newClient().makefile()
        greeting = c1.readline()

        self.assertTrue(greeting.startswith("Welcome"), "Greeting must start with 'Welcome'")
        c1.close()

    def test_greeting1(self):
        c1 = self.newClient().makefile()
        c2 = self.newClient().makefile()

        g1 = c1.readline()
        g2 = c2.readline()

        self.assertTrue(g1.startswith("Welcome"), "Greeting must start with 'Welcome'")
        self.assertTrue(g2.startswith("Welcome"), "Greeting must start with 'Welcome'")

        c1.close()
        c2.close()

class Test5(TestBase):
    def test_echo(self):
        c1 = self.newClient()
        c1f = c1.makefile()
        c1f.readline()

        c1.sendall("tes")
        c1.sendall("t1\ntest2\ntest3\n")

        for msg in ["test1\n", "test2\n", "test3\n"]:
            l = c1f.readline()
            self.assertTrue( l.endswith(msg), "Received message '{0}', expecting '{1}'".format(l, msg))

        c1.close()

class Test6(TestBase):
    def test_communicate(self):
        c1 = self.newClient()
        c1f = c1.makefile()
        c2 = self.newClient()
        c2f = c2.makefile()

        c1f.readline()
        c2f.readline()

        c1.sendall("test1\ntest2\n") 
        waitFor(lambda: self.reader.countString("test2") == 1)

        c2.sendall("test")
        c2.sendall("3\ntest4\n") 

        msgs = ["test1\n", "test2\n", "test3\n", "test4\n"]
        for msg in msgs:
            l = c1f.readline()

            self.assertTrue( l.endswith(msg), "Invalid message received message '{0}', expecting '{1}'".format(l, msg))

        for msg in msgs:
            l = c2f.readline()
            self.assertTrue( l.endswith(msg), "Invalid message received message '{0}', expecting '{1}'".format(l, msg))


        c1.close()
        c2.close()


if __name__ == '__main__':
    unittest.main()


