TEST_TARGET_DIRS = resea tests

test:
	py.test -v --doctest-modules $(TEST_TARGET_DIRS)

debug:
	py.test -v --doctest-modules --pdb $(TEST_TARGET_DIRS)

install:
	make clean
	python3 setup.py install
	make clean

clean:
	rm -rf build dist Resea.egg-info hello

pep8:
	pep8 --ignore=E302,E501,E241 .


register:
	python3 setup.py register

release:
	if [ "$(RELEASE_VERSION)" = "" ] ; then \
        echo "error: set RELEASE_VERSION"; \
        exit 1 ; \
    fi
	echo "__version__ = '$(RELEASE_VERSION)'" > resea/version.py
	git add .
	git commit -m "version $(RELEASE_VERSION)"
	git tag "v$(RELEASE_VERSION)"

upload:
	python3 setup.py sdist upload
	make clean
	git push
	git push --tags
