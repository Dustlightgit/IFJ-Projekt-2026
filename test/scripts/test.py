import sys
import subprocess
from pathlib import Path

# Format polozky: ["COMPONENT", "test1", "test2", ...]
INTERPRET_TESTS = [
    ["generator", "example_program", "sem_valid_program"],
    ["ifj26_to_bin", "algebdt", "boolthen", "control", "extkw", "factorial", "funexp", "interpolateds", "lambdas", "lists", "paramlists", "strings", "test_ahoj", "valmem"],
]

EXCEPTIONS = [
    ["ifj26_to_bin", "algebdt", "extkw", "interpolateds", "lambdas", "paramlists"],
]


def is_in_test_list(test_list, component_name, test_name):
    return any(
        entry and entry[0] == component_name and test_name in entry[1:]
        for entry in test_list
    )


def run_tests():
    scripts_dir = Path(__file__).parent.resolve()
    interpret_bin = scripts_dir.parent / "interpret" / "ic26int-linux-x86_64"

    selected_component = None
    selected_test = None

    # Argumenty: python3 test.py [COMPONENT] [TEST]
    if len(sys.argv) > 1:
        selected_component = sys.argv[1]

    if len(sys.argv) > 2:
        selected_test = sys.argv[2]

    # Vyhledani komponent
    if selected_component:
        component_dirs = [scripts_dir / selected_component]

        if not component_dirs[0].is_dir():
            print(
                f"ERROR: Komponenta '{selected_component}' neexistuje.",
                file=sys.stderr
            )
            sys.exit(1)
    else:
        component_dirs = sorted(
            d for d in scripts_dir.iterdir()
            if d.is_dir() and (d / "build" / "runner").exists()
        )

    if not component_dirs:
        print("ERROR: Nebyly nalezeny zadne komponenty.")
        sys.exit(1)

    total_passed = 0
    total_failed = 0

    for component_dir in component_dirs:
        component_name = component_dir.name
        runner_bin = component_dir / "build" / "runner"
        unit_tests_dir = component_dir / "unit_tests"

        if not runner_bin.exists():
            print(
                f"[SKIP] {component_name}: runner neexistuje."
            )
            continue

        if not unit_tests_dir.is_dir():
            print(
                f"[SKIP] {component_name}: unit_tests neexistuje."
            )
            continue

        test_dirs = sorted(
            d for d in unit_tests_dir.iterdir()
            if d.is_dir() and (
                selected_test is None or d.name == selected_test
            )
        )

        if selected_test and not test_dirs:
            print(
                f"[ERROR] Test '{selected_test}' "
                f"nebyl nalezen v komponente '{component_name}'."
            )
            total_failed += 1
            continue

        print(f"\n========== {component_name} ==========")

        for test_dir in test_dirs:
            if is_in_test_list(EXCEPTIONS, component_name, test_dir.name):
                continue

            input_file = test_dir / "input.txt"
            stdin_file = test_dir / "stdin.txt"
            expected_file = test_dir / "expected.txt"
            output_file = test_dir / "output.txt"

            if not input_file.exists() or not expected_file.exists():
                print(
                    f"[SKIP] {test_dir.name}: "
                    "chybi input.txt nebo expected.txt"
                )
                continue

            use_interpret = is_in_test_list(INTERPRET_TESTS, component_name, test_dir.name)

            test_stdin = (
                stdin_file.read_text(encoding="utf-8")
                if stdin_file.exists()
                else None
            )

            try:
                with open(input_file, "r", encoding="utf-8") as infile:
                    result = subprocess.run(
                        [str(runner_bin)],
                        stdin=infile,
                        capture_output=True,
                        text=True,
                        check=False
                    )

                if result.returncode == 0 and use_interpret:
                    ifjcode_file = test_dir / "ifjcode.txt"
                    ifjcode_file.write_text(
                        result.stdout,
                        encoding="utf-8"
                    )
                    result = subprocess.run(
                        [str(interpret_bin), str(ifjcode_file)],
                        input=test_stdin,
                        capture_output=True,
                        text=True,
                        check=False
                    )

                parts = [p.rstrip("\n") for p in (result.stdout, result.stderr) if p]
                if result.returncode != 0:
                    parts.append(f"EXIT CODE: {result.returncode}")

                output_text = "\n".join(parts) + ("\n" if parts else "")
                output_file.write_text(output_text, encoding="utf-8")

            except OSError as error:
                print(f"\033[33m[FAIL] {test_dir.name}\033[0m")
                print(f"  - Chyba: {error}")
                total_failed += 1
                continue

            expected_lines = [
                line.strip()
                for line in expected_file.read_text(
                    encoding="utf-8"
                ).splitlines()
                if line.strip()
            ]

            actual_lines = [
                line.strip()
                for line in output_text.splitlines()
                if line.strip()
            ]
            returncodemsg = ""
            if result.returncode != 0:
                returncodemsg = f"\033[90m  - Proces skoncil s kodem {result.returncode}\033[0m"
            if actual_lines == expected_lines:
                print(f"\033[92m[OK]\033[0m   {test_dir.name}      {returncodemsg}")
                total_passed += 1
            else:
                print(f"\033[91m[FAIL]\033[0m {test_dir.name}      {returncodemsg}")
                total_failed += 1

                max_lines = max(
                    len(actual_lines),
                    len(expected_lines)
                )

                for i in range(max_lines):
                    expected = (
                        expected_lines[i]
                        if i < len(expected_lines)
                        else "<konec souboru>"
                    )

                    actual = (
                        actual_lines[i]
                        if i < len(actual_lines)
                        else "<konec souboru>"
                    )

                    if expected != actual:
                        print(
                            f"  - Radek {i + 1}: "
                            f"Ocekavano '{expected}', "
                            f"Ziskano '{actual}'"
                        )

                print(
                    f"  - Pocet radku: "
                    f"Ocekavano {len(expected_lines)}, "
                    f"Ziskano {len(actual_lines)}"
                )

    print(
        f"\nCelkovy vysledek: "
        f"{total_passed} uspesnych, "
        f"{total_failed} selhalo."
    )

    if total_failed > 0:
        sys.exit(1)


if __name__ == "__main__":
    run_tests()