import os
import yaml
import pytest
import resea
import subprocess


@pytest.fixture(scope='session')
def package(request):
    """ Creates a package named hello. """

    if not os.path.exists('hello'):
        resea.main(['new', 'hello'])

    os.chdir('hello')

    package_yml = yaml.load(open('package.yml'))
    package_yml['name'] = 'hello'
    package_yml['category'] = 'application'
    package_yml['lang'] = 'cpp'
    package_yml['depends'] = ['cpp']
    package_yml['config'] = {'SOURCES': {'set': ['src/startup.cpp', 'src/test.cpp']}}
    yaml.dump(package_yml, open('package.yml', 'w'))

    open('src/startup.cpp', 'w').write("""\
#include "hello.h"
#include <resea.h>

extern "C" void hello_startup(void){

  INFO("Hello, World!");
  for (;;);
}
""")

    open('src/hello.h', 'w').write("""\
#ifndef __HELLO_HELLO_H__
#define __HELLO_HELLO_H__

#define PACKAGE_NAME "hello"

#endif
""")

    open('src/test.cpp', 'w').write("""\
#include "hello.h"
#include <resea.h>

extern "C" void hello_test(void) {

  TEST_EXPECT(123 == 124-1);
  TEST_END();
}
""")

    def fin():
        os.chdir('..')
    request.addfinalizer(fin)
