project('cppargparser', 'cpp')

src_path = files('src/arg_parser.cpp')
hdr_path = include_directories('include')

lib_so = shared_library('cppargparser', sources : src_path,
                        include_directories : hdr_path,
                        install : true)
lib_stat = static_library('cppargparser', sources : src_path,
                          include_directories : hdr_path,
                          install : true)

install_headers('include/arg_parser.hpp', subdir : 'cppargparser')

tests = [
    executable('test-option-register',
               sources : 'unit-tests/test-option-register.cpp',
               include_directories : hdr_path,
               link_with : lib_stat),

    executable('test-option-find',
               sources : 'unit-tests/test-option-find.cpp',
               include_directories : hdr_path,
               link_with : lib_stat),

    executable('test-positional-find',
               sources : 'unit-tests/test-positional-find.cpp',
               include_directories : hdr_path,
               link_with : lib_stat),

    executable('test-parse-option',
               sources : 'unit-tests/test-parse-option.cpp',
               include_directories : hdr_path,
               link_with : lib_stat),

    executable('test-mtx-options',
               sources : 'unit-tests/test-mtx-options.cpp',
               include_directories : hdr_path,
               link_with : lib_stat)
]

test('OptionRegistration', tests[0])
test('OptionFind', tests[1], args : ['--useful-option'])
test('PositionalFind', tests[2], args : ['1'])
test('ParseOption', tests[3], args : ['--int', '1', '--hex', 'FF', '--string', 'Hello', '--float', '0.1'])
test('MutualExclusion', tests[4], args : ['-a'])
test('MutualExclusion2Groups', tests[4], args : ['-a', '-b'])
test('MutualExclusionConflict', tests[4], args : ['-a', '-b', '-c'], should_fail : true)
