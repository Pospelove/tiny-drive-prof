# Header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO bshoshany/thread-pool
    REF 910f7cd95b53af49e2512bcaa2c8335944fa3b2f
    SHA512 c6eb28e805a1b97d3993c00465e98983263eb41219e7c6c3628770a139a02836ac159ec67c268d5ad32dfeefdae66d95d3ae8f8c4eba2f747b980982cd286a68
    HEAD_REF master
)

file(
    COPY ${SOURCE_PATH}/thread_pool.hpp
    DESTINATION ${CURRENT_PACKAGES_DIR}/include
)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)