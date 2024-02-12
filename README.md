[![Ensure compliance with Ledger guidelines](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/guidelines_enforcer.yml/badge.svg)](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/guidelines_enforcer.yml) [![Build and run functional tests using ragger through reusable workflow](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/build_and_functional_tests.yml/badge.svg?branch=master)](https://github.com/partisiablockchain/pbc-ledger-embedded-app/actions/workflows/build_and_functional_tests.yml)

# Partisia Blockchain Ledger Embedded Application

![](./documentation/images/ManagerIcon.svg)

Ledger Application for interacting with Partisia Blockchain.

Based on [Ledger app-boilerplate](https://github.com/LedgerHQ/app-boilerplate),
commit `7d1c3c39d8c7d3567c5a6650fe339a8e5160b497`.

## Changes from Ledger App Boilerplate

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

### Loading on a physical device

From the above-mentioned docker container, run `make load`.

:information_source: Your physical device must be connected, unlocked and the screen showing the dashboard (not inside an application).

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
