[![Ensure compliance with Ledger guidelines](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/guidelines_enforcer.yml/badge.svg)](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/guidelines_enforcer.yml)
[![Build and run functional tests using ragger through reusable workflow](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/build_and_functional_tests.yml/badge.svg)](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/build_and_functional_tests.yml)

# ![Partisia Blockchain SVG Logo](./documentation/images/ManagerIcon.svg) Partisia Blockchain Ledger Embedded Application

Ledger Application for interacting with Partisia Blockchain.

Supported use cases:

- Clear-signing of [MPC Token](https://partisiablockchain.gitlab.io/documentation/pbc-fundamentals/governance-system-smart-contracts-overview.html#mpc-tokens) transfers, with and without memos.
- Blind-singing of arbitrary transactions of any size on [Partisia Blockchain](https://browser.partisiablockchain.com/)
  and [Partisia Blockchain Testnet](https://browser.testnet.partisiablockchain.com/). This functionality may put your crypto
  assets at risk, and must be explicitly enabled through the settings menu.

## Changes

Based on [Ledger app-boilerplate](https://github.com/LedgerHQ/app-boilerplate),
commit `7d1c3c39d8c7d3567c5a6650fe339a8e5160b497`.

- Targets Partisia Blockchain, with clear signing support for MPC Token, and blind signing support for all other transactions.
- Adjusted rest of the code base to fit this target, including, but not limited
  to:
  * Parsing logic
  * Signing logic
  * APDU format
  * UI and ícons
  * Unit Testing
  * Functional Testing
  * Fuzz Testing
- Documentation has been updated to reflect Partisia Blockchain.

## Quick start guide

Trimmed and modified from [Ledger app-boilerplate README](https://github.com/LedgerHQ/app-boilerplate).

### Setup environment

The [ledger-app-dev-tools](https://github.com/LedgerHQ/ledger-app-builder/pkgs/container/ledger-app-builder%2Fledger-app-dev-tools) docker image contains all the required tools and libraries to **build**, **test** and **load** an application.

You can download it from the ghcr.io docker repository:

```shell
sudo docker pull ghcr.io/ledgerhq/ledger-app-builder/ledger-app-dev-tools:latest
```

You can then enter this development environment by executing the following command from the directory of the application `git` repository:

**Linux (Ubuntu)**

```shell
sudo docker run --rm -ti --user root --privileged -v "/dev/bus/usb:/dev/bus/usb" -v "$(realpath .):/app" ghcr.io/ledgerhq/ledger-app-builder/ledger-app-dev-tools:latest
```

**Windows (with PowerShell)**

```shell
docker run --rm -ti --privileged -v "$(Get-Location):/app" ghcr.io/ledgerhq/ledger-app-builder/ledger-app-dev-tools:latest
```

The application's code will be available from inside the docker container, you can proceed to the following compilation steps to build your app.

### Compilation

Setup a compilation environment by following the [shell with docker approach](#with-a-terminal).

From inside the container, use the following command to build the app for all
supported Ledger devices:

```shell
bash scripts/compile.sh
```

### Test application on a physical device

You can test the application on a physical device by loading it onto the device.

Before you can load the application the prerequisite steps must have been follows:

1. Compile the application to the target device. The artifacts are located in `./build/<target>`.
   
   The target for the _Nano S Plus_ is located in the `./build/nanos2` folder.

2. Connect the test device and unlock it. It must display the dashboard and not the application itself.

With these two steps, the application loading can be initialized in one of the two following ways.

#### Using Visual Studio Code extension

1. Open the pbc-ledger-embedded-app project folder in Visual Studio Code
2. Install the [Ledger Dev Tools](https://marketplace.visualstudio.com/items?itemName=LedgerHQ.ledger-dev-tools) extension
3. Open the Ledger Developer Tools menu located on the Primary Side Bar (usually located on the left side)
4. Press the "Select Target" option and choose your target device
5. Under "Device Operations" press the "Load app on device" option

#### Using the command line

Inside the docker container run command,
   
```shell
python3 -m ledgerblue.runScript  --scp --fileName build/<target>/bin/app.apdu --elfFile build/<target>/bin/app.elf
```

Where `<target>` is replaced with your target device, e.g. `nanos2`.

#### Finalize loading the application

The terminal should now display two messages, "Generated random root public key" and "Using test master key".

The next steps must be done on the device itself.

1. When the device displays the message "Deny unsafe manager", go through the screens and select the "Allow unsafe manager" option.
2. When the device displays the message "Uninstall Partisia Blockchain", go through the screens and select the "Confirm action" option.
3. When the device displays the message "Install app Partisia Blockchain", go through the screens and select "Perform installation" option.
4. Enter the device pin

## Test

The app comes with functional tests implemented with Ledger's [Ragger](https://github.com/LedgerHQ/ragger) test framework.

### Linux

Install the tests requirements:

```shell
pip install -r tests/requirements.txt
```

Then run tests for all Ledger devices using:

```shell
bash scripts/test-ui.sh
```

## Documentation

High level documentation such as [application specification](documentation/PBC_APP_APDU.md), [the APDU format in general](documentation/APDU.md) and [transaction serialization](documentation/TRANSACTION.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

```shell
doxygen .doxygen/Doxyfile
```

the process outputs HTML and LaTeX documentations in `doc/html` and `doc/latex` folders.
