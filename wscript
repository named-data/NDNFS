# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION='0.2'
APPNAME='NDNFS'

from waflib import Build, Logs, Utils, Task, TaskGen, Configure

def options(opt):
    opt.add_option('--debug',action='store_true',default=False,dest='debug',help='''debugging mode''')
    opt.add_option('--test', action='store_true',default=False,dest='_test',help='''build unit tests''')

    # if Utils.unversioned_sys_platform () == "darwin":
    #     pass

    opt.load('compiler_c compiler_cxx boost ccnx mongodb')

def configure(conf):
    conf.load("compiler_c compiler_cxx")

    if conf.options.debug:
        conf.define ('NDNFS_DEBUG', 1)
        conf.add_supported_cxxflags (cxxflags = ['-O0',
                                                 '-Wall',
                                                 '-Wno-unused-variable',
                                                 '-g3',
                                                 '-Wno-unused-private-field', # only clang supports
                                                 '-fcolor-diagnostics',       # only clang supports
                                                 '-Qunused-arguments'         # only clang supports
                                                 ])
    else:
        conf.add_supported_cxxflags (cxxflags = ['-O3', '-g'])

    conf.define ("FUSE_NDNFS_VERSION", VERSION)

    try:
        conf.check_cfg(package='osxfuse', args=['--cflags', '--libs'], uselib_store='FUSE', mandatory=True)
        conf.define("NDNFS_OSXFUSE", 1)
    except:
        try:
            conf.check_cfg(package='fuse', args=['--cflags', '--libs'], uselib_store='FUSE', mandatory=True)
            conf.define("NDNFS_FUSE", 1)
        except:
            conf.fatal ("Cannot find FUSE libraries")

    conf.check_cfg(package='sqlite3', args=['--cflags', '--libs'], uselib_store='SQLITE3', mandatory=True)

    # if Utils.unversioned_sys_platform () == "darwin":
    #     pass

    conf.write_config_header('config.h')

    if not conf.check_cfg(package='openssl', args=['--cflags', '--libs'], uselib_store='SSL', mandatory=False):
        libcrypto = conf.check_cc(lib='crypto',
                                  header_name='openssl/crypto.h',
                                  define_name='HAVE_SSL',
                                  uselib_store='SSL')
    else:
        conf.define ("HAVE_SSL", 1)
    if not conf.get_define ("HAVE_SSL"):
        conf.fatal ("Cannot find SSL libraries")

    conf.load ('ccnx')
    conf.check_cfg(package='libndn.cxx', args=['--cflags', '--libs'], uselib_store='NDNCXX', mandatory=True)

    conf.load('boost')
    conf.check_boost(lib='system test iostreams filesystem thread')

    conf.check_ccnx (path=conf.options.ccnx_dir)
    conf.define ('CCNX_PATH', conf.env.CCNX_ROOT)

    if conf.options._test:
        conf.define ('_TESTS', 1)
        conf.env.TEST = 1

def build (bld):
    bld (
        target = "ndnfs",
        features = ["cxx", "cxxprogram"],
        source = bld.path.ant_glob(['fs/*.cc']),
        use = 'BOOST BOOST_SYSTEM BOOST_FILESYSTEM BOOST_THREAD FUSE CCNX SSL NDNCXX SQLITE3',
        includes = ".",
        )
    bld (
        target = "ndnfs-server",
        features = ["cxx", "cxxprogram"],
        source = bld.path.ant_glob(['server/server.cc', 'server/servermodule.cc']),
        use = 'BOOST BOOST_SYSTEM BOOST_FILESYSTEM BOOST_THREAD CCNX SSL NDNCXX SQLITE3',
        includes = ".",
        )
    # bld (
    #     target = "test-client",
    #     features = ["cxx", "cxxprogram"],
    #     source = bld.path.ant_glob(['test/client.cc']),
    #     use = 'BOOST BOOST_SYSTEM BOOST_FILESYSTEM BOOST_THREAD CCNX SSL NDNCXX',
    #     includes = ".",
    #     )

@Configure.conf
def add_supported_cxxflags(self, cxxflags):
    """
    Check which cxxflags are supported by compiler and add them to env.CXXFLAGS variable
    """
    self.start_msg('Checking allowed flags for c++ compiler')

    supportedFlags = []
    for flag in cxxflags:
        if self.check_cxx (cxxflags=[flag], mandatory=False):
            supportedFlags += [flag]

    self.end_msg (' '.join (supportedFlags))
    self.env.CXXFLAGS += supportedFlags
