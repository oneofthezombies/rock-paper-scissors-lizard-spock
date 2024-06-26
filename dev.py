import sys
import shutil
import os
import subprocess


def run(fp, command):
    with subprocess.Popen(
        command, stdout=subprocess.PIPE, text=True, bufsize=1, universal_newlines=True
    ) as proc:
        for line in proc.stdout:
            print(line, end="")
            fp.write(line)
        proc.wait()
        if proc.returncode != 0:
            raise subprocess.CalledProcessError(proc.returncode, command)


def clean():
    if os.path.exists("build"):
        shutil.rmtree("build")
    if os.path.exists("local"):
        shutil.rmtree("local")


def build():
    with open("build.log", "w") as f:
        run(
            f,
            [
                "cmake",
                "-S",
                ".",
                "-B",
                "build",
                "-G",
                "Ninja",
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                f"-DCMAKE_INSTALL_PREFIX={os.path.join(os.getcwd(), 'local')}",
            ],
        )
        run(f, ["cmake", "--build", "build"])
        run(f, ["cmake", "--install", "build"])



def print_help():
    print("Usage: python dev.py <command>")
    print("Commands:")
    print("  clean        - Clean the project")
    print("  build        - Build the project")


def main():
    args = sys.argv[1:]
    command = args[0] if args else "help"

    if command == "clean":
        clean()
    elif command == "build":
        build()
    else:
        print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
