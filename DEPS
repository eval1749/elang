# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'buildtools_revision': '15308f469a704c45d15567fa69cd94ce07ad0e1b',
}

deps = {
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446b273697fadf3a10ff1007a97c0b7de6df',

  'src/tools/deps2git':
    Var('chromium_git') + '/chromium/tools/deps2git.git' + '@' + 'f04828eb0b5acd3e7ad983c024870f17f17b06d9',

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + '29763965ab52f24565299976b936d1265cb6a271', # from svn revision 501

  'src/testing/gtest':
    Var('chromium_git') + '/external/googletest.git' + '@' + 'be1868139ffe0ccd0e8e3b37292b84c821d9c8ad', # from svn revision 704

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + '2a5511bd901f328db10d0b6415c864a5ff59fc81',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '10834e84d250eb31a73496bfdb7923eaa43ead51',

}

hooks = [
  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'pattern': '.',
    'action': ['src\\elang\\build\\gn_elang.cmd']
  }
]
