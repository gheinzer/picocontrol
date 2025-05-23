import os
import subprocess
import mimetypes
from SCons.Script import *

env = DefaultEnvironment()

FS_BINARY_NAME = "webfs.bin"
FS_BINARY_PATH = os.path.join(env.subst("$BUILD_DIR"), FS_BINARY_NAME)
FS_BINARY_SOURCE_FOLDER = "./web"

def buildFsBinary(source, target):
    print("Writing version file...")
    with open(os.path.join(source, "version.txt"), "w+") as versionFile:
        # Get version from git
        try:
            version = subprocess.check_output(["git", "describe", "--tags"]).strip().decode("utf-8")
        except subprocess.CalledProcessError:
            version = "version could not be determined at build time"
        versionFile.write(version)

    print("Building file system binary...")

    fileTable = {}

    for root, dirs, files in os.walk(str(source)):
        for name in files:
            path = os.path.join(root, name)
            relpath = os.path.relpath(path, source)
            fileTable[relpath] = os.path.getsize(path)

    with open(str(target), "+wb") as fp:
        # Write the number of files as uint32
        fp.write(int(len(fileTable)).to_bytes(4, "little", signed = False))

        # Write the file table
        for path, size in fileTable.items():
            pathTerminated = path.encode("ascii") + b'\x00'
            fp.write(pathTerminated)

            mimetype, encoding = mimetypes.guess_type(path)
            mimeTypeTerminated = (mimetype or "text/plain").encode("ascii") + b'\x00'
            fp.write(mimeTypeTerminated)

            # Align to the next 4-byte boundary
            paddingCount = 4 - (fp.tell() % 4)
            fp.write(paddingCount * b'\x00')
            fp.write(int(size).to_bytes(4, "little", signed = False)) # Write file size as uint32

        for path in fileTable:
            with open(os.path.join(source, path), "rb") as src:
                fp.write(src.read())

    print("File system binary built successfully.")

buildFsBinary(FS_BINARY_SOURCE_FOLDER, FS_BINARY_PATH)