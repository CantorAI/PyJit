#!/bin/bash
set -e -u -x

export RELEASE_VERSION=$1
release_folder=$2

manylinux="${release_folder}/manylinux"
wheelhouse="${release_folder}/wheelhouse"

echo ${manylinux}
echo ${wheelhouse}

rm -rf ${wheelhouse}
rm -rf ${manylinux}

function repair_wheel {
    wheel="$1"
    if ! auditwheel show "$wheel"; then
        echo "Skipping non-platform wheel $wheel"
    else
        auditwheel repair "$wheel"  -w ${manylinux}
    fi
}


python_cps=("/opt/python/cp38-cp38/bin" \
        "/opt/python/cp39-cp39/bin" \
        "/opt/python/cp310-cp310/bin" \
        )

# Compile wheels
for PYBIN in "${python_cps[@]}"; do
    "${PYBIN}/pip" install -r ./dev-requirements.txt
    "${PYBIN}/pip" wheel ../ --no-deps -w ${wheelhouse}
done

# Bundle external shared libraries into the wheels
for whl in ${wheelhouse}/*.whl; do
    repair_wheel "$whl"
done

# Install packages and test
for PYBIN in "${python_cps[@]}"; do
    "${PYBIN}/pip" install pyjit --no-index -f ${wheelhouse}
    #(cd "$HOME"; "${PYBIN}/nosetests" pymanylinuxdemo)
done
