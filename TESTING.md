# Testing

This is a comprehensive guide to the automated test suites included in Manticore Search. Testing ensures that new changes are stable, free of bugs, and don't introduce regressions. This document covers the test frameworks used in Manticore Search: regression tests (PHP-based), Google Tests (C++ unit tests), and CLT (Command Line Testing) tests.

## Table of Contents

- [Overview](#overview)
- [Test Types](#test-types)
- [Testing via GitHub Actions](#testing-via-github-actions)
- [Local Testing Setup](#local-testing-setup)
  - [Requirements](#requirements)
  - [Platform-Specific Setup](#platform-specific-setup)
- [Running CTest Tests](#running-ctest-tests)
- [Running CLT Tests](#running-clt-tests)
- [Writing Tests](#writing-tests)
- [Test Structure](#test-structure)

## Overview

Manticore Search uses several types of automated tests:

1. **Regression Tests** - PHP-based functional tests located in `test/test_XXX/` directories, executed via `ubertest.php`
2. **Google Tests** - C++ unit tests for core functionality located in `src/gtests/`
3. **CLT Tests** - Command-line integration tests in `test/clt-tests/` using the [CLT framework](https://github.com/manticoresoftware/clt)

CTest (CMake's testing tool) is used to run regression tests and Google Tests locally.

## Test Types

### Regression Tests
PHP-based functional tests located in `test/test_XXX/` directories. These tests verify searchd functionality by comparing actual output against reference results. Executed via `ubertest.php` and run through CTest.

### Google Tests
C++ unit tests located in `src/gtests/`. These tests verify core C++ functionality, including tokenizers, JSON processing, filters, and internal data structures. Run via CTest.

### CLT Tests
Located in `test/clt-tests/` and organized by functionality. Key categories include:

- **base/** - Reusable blocks for starting/stopping searchd
- **buddy-plugins/** - Tests for Buddy plugins (fuzzy search, autocomplete, etc.)
- **expected-errors/** - Negative tests validating error handling
- **fulltext-search/** - Full-text search functionality tests
- **performance/** - Performance benchmarks
- **replication/** - Replication and clustering tests
- **sharding/** - Distributed table and sharding tests
- **vector-knn/** - Vector/KNN search tests

See `test/clt-tests/` directory for complete list of test categories.

## Testing via GitHub Actions

**Recommended for most contributors**: Instead of setting up a local test environment, you can use GitHub Actions to run tests automatically.

### How it works:

1. **Fork the repository** on GitHub
2. **Enable GitHub Actions** in your fork: Go to the "Actions" tab and click "I understand my workflows, go ahead and enable them"
3. **Create a branch** in your fork
4. **Make your changes** and commit them
5. **Push to your fork** - This automatically triggers GitHub Actions
6. **Create a Pull Request** - Tests run automatically on PR creation and updates

GitHub Actions will run the full test suite across multiple platforms (Linux, macOS, Windows) and report results directly in your PR.

### Viewing Test Results:

- Navigate to the "Actions" tab in your forked repository
- Click on your workflow run to see test results
- Test failures will be highlighted with details

This approach is recommended because:
- No local setup required
- Tests run on multiple platforms automatically
- Same environment as CI/CD
- Test results are preserved and shareable

## Local Testing Setup

For contributors who need to run tests locally or develop new tests.

### Requirements

#### All Platforms

* **Build tools**: Follow [Compiling from Sources](https://manual.manticoresearch.com/Installation/Compiling_from_sources) guide
* **PHP**: Version 7.4+ with extensions: `mysql`, `curl`, `xml`, `mbstring`
* **MySQL/MariaDB**: For running tests that require SQL database connectivity
* **Docker**: For running CLT tests in containerized environment (recommended)

#### Optional (for specific test categories)

* **Python**: For utility scripts
* **jq**: For JSON processing in tests
* **Kafka**: For integration tests (provided via Docker)

### Platform-Specific Setup

#### Linux (Ubuntu/Debian)

```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libmysqlclient-dev php php-mysql php-curl php-xml php-mbstring mysql-server docker.io jq

# Configure MySQL for tests
sudo mysql -e "CREATE DATABASE IF NOT EXISTS test;"
sudo mysql -e "CREATE USER IF NOT EXISTS 'test'@'localhost';"
sudo mysql -e "GRANT ALL PRIVILEGES ON test.* TO 'test'@'localhost';"
sudo mysql -e "FLUSH PRIVILEGES;"
```

#### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake mysql php jq
brew install --cask docker

# Start MySQL
brew services start mysql

# Configure MySQL for tests
mysql -u root -e "CREATE DATABASE IF NOT EXISTS test;"
mysql -u root -e "CREATE USER IF NOT EXISTS 'test'@'localhost';"
mysql -u root -e "GRANT ALL PRIVILEGES ON test.* TO 'test'@'localhost';"
mysql -u root -e "FLUSH PRIVILEGES;"
```

#### Windows

For Windows, we strongly recommend using WSL2 (Windows Subsystem for Linux) and following the Linux instructions above, or using Docker Desktop for Windows.

Alternatively, use GitHub Actions for testing (see [Testing via GitHub Actions](#testing-via-github-actions)).

### Custom MySQL Configuration

If you need custom MySQL connection settings, create a `.sphinx` file in your home directory:

```php
<?php
return array(
    "db-host" => "127.0.0.1",
    "db-user" => "myuser",
    "db-name" => "mydb",
    "db-password" => "mypass",
    "lemmatizer_base" => "/path/to/aot_packs"
);
```

## Running CTest Tests

Traditional test suite using CMake/CTest:

```bash
# Build Manticore
mkdir build && cd build
cmake ..
make -j8

# Run all tests
ctest -C Debug

# Run specific test by name (regex)
ctest -C Debug -R test_101

# Run range of tests
ctest -C Debug -I 1,10,1

# Run with verbose output
ctest -C Debug -V

# Run tests in parallel
ctest -C Debug -j8
```

## Running CLT Tests

CLT (Command Line Testing) is the modern testing framework for Manticore Search. CLT tests are primarily run via GitHub Actions, but can also be run locally using the CLT Docker action.

### Running via GitHub Actions (Recommended)

The easiest way to run CLT tests is through GitHub Actions:

1. Push your changes to a branch in your fork
2. GitHub Actions will automatically run all CLT test suites
3. View results in the "Actions" tab of your repository

CLT tests are organized into test suites defined in `.github/workflows/clt_tests.yml`.

### Running Locally

CLT tests run inside Docker containers to ensure consistent environment. There are two main scenarios for local testing:

#### 1. Testing Against Public Images (Quick Verification)

For quick testing of existing functionality using public Docker images:

```bash
# Pull the latest Manticore image from Docker Hub
docker pull manticoresearch/manticore:latest

# Or use the dev version with latest features
docker pull manticoresearch/manticore:dev

# Start Manticore container
docker run -d --name manticore-test \
  -p 9306:9306 -p 9308:9308 \
  manticoresearch/manticore:latest

# Test connection
mysql -h127.0.0.1 -P9306 -e "SHOW TABLES;"
```

**Running CLT tests against public images:**

```bash
# Clone CLT repository
git clone https://github.com/manticoresoftware/clt.git
cd clt

# Run a specific test
./clt test -t /path/to/manticoresearch/test/clt-tests/buddy-plugins/test-fuzzy-search.rec \
  -d manticoresearch/manticore:latest

# Record a new test interactively
./clt record manticoresearch/manticore:latest
```

See [CLT documentation](https://github.com/manticoresoftware/clt) for more details on test recording and replay.

#### 2. Testing Your Own Code Changes

When you've made changes to Manticore Search code and want to test them before creating a PR, use the [`build-local-test-kit.sh`](https://github.com/manticoresoftware/manticoresearch/blob/master/misc/build-local-test-kit.sh) script.

**Usage:**

```bash
cd misc
./build-local-test-kit.sh          # Build image with your changes (~10-30 min)
./build-local-test-kit.sh --clean  # Clean build from scratch
./build-local-test-kit.sh --help   # Show detailed help
```

**What it does:**

1. Pulls base image `test-kit-latest` (contains latest dev from master)
2. Compiles your code changes
3. Creates Docker image `test-kit:local` ready for CLT testing

**Example workflow:**

```bash
# 1. Make code changes
vim src/searchdhttp.cpp

# 2. Build test-kit with your changes
cd misc && ./build-local-test-kit.sh

# 3. Run CLT tests against your image
cd /path/to/clt
./clt test -t /path/to/test.rec -d test-kit:local

# 4. Tests pass! Commit and push
git add . && git commit -m "Add feature" && git push
```

**Options:**

- `BUILD_TYPE=Debug ./build-local-test-kit.sh` - Build in Debug mode
- Supports Linux, macOS (Intel/Apple Silicon), Windows (WSL2)
- See `./build-local-test-kit.sh --help` for full documentation

GitHub Actions will automatically build and test your PR the same way.

### CLT Test File Format

CLT uses two types of files:

- **`.rec`** (test case) - Complete test file that CLT executes. Contains test steps and can include reusable blocks.
- **`.recb`** (reusable block) - Shared test component that can be included in multiple `.rec` files using `––– block: path/to/block –––`

**Example `.rec` file** (complete test):

```
––– block: ../base/start-searchd-with-buddy –––
––– input –––
mysql -h0 -P9306 -e "CREATE TABLE test(content TEXT);"
––– output –––
––– input –––
mysql -h0 -P9306 -e "SELECT * FROM test;"
––– output –––
+------+---------+
| id   | content |
+------+---------+
```

The first line includes a reusable block from `../base/start-searchd-with-buddy.recb` which contains common setup steps (starting searchd and Buddy).

**When to use each:**
- Use `.rec` for your actual test cases
- Use `.recb` for common setup/teardown steps that multiple tests need (see [Base Blocks](#base-blocks) section below)

See existing tests in `test/clt-tests/` for more examples.

## Writing Tests

### Adding a CTest Test

1. Create a new directory: `test/test_XXX/`
2. Add test data and configuration files
3. The test will be automatically discovered by CMake

### Adding a CLT Test

1. Choose appropriate category in `test/clt-tests/`
2. Create a `.rec` file with your test
3. Use base blocks from `test/clt-tests/base/` for common setup
4. Follow format of existing tests

Example:
```bash
# Create new test
cat > test/clt-tests/buddy-plugins/test-my-feature.rec << 'EOF'
––– block: ../base/start-searchd-with-buddy –––
––– input –––
mysql -h0 -P9306 -e "SELECT 1;"
––– output –––
+------+
| 1    |
+------+
| 1    |
+------+
EOF

# Run your test via GitHub Actions by pushing to your fork
git add test/clt-tests/buddy-plugins/test-my-feature.rec
git commit -m "Add test for my feature"
git push origin your-branch
```

### Test Guidelines

- **Always add tests for bug fixes**: If fixing a regression, add a test that fails before the fix and passes after
- **Test both positive and negative cases**: Include tests in `expected-errors/` for error conditions
- **Use descriptive names**: `test-fuzzy-search-with-layouts.rec` is better than `test1.rec`
- **Keep tests focused**: One test should verify one specific behavior
- **Document complex tests**: Add comments explaining non-obvious test logic

## Test Structure

### Base Blocks

Reusable test components in `test/clt-tests/base/`:

- `start-searchd.recb` - Start searchd without Buddy
- `start-searchd-with-buddy.recb` - Start searchd with Buddy plugin system
- `start-searchd-with-cpustats.recb` - Start with CPU monitoring
- `start-kafka-kraft.recb` - Start Kafka for integration tests
- `basic-initialization-manticore-kafka.recb` - Initialize Manticore with Kafka
- `kafka-reset-consumer-offsets.recb` - Reset Kafka consumer offsets
- `searchd-with-flexible-ports.conf` - Configuration with flexible port settings
- `replication/` - Reusable blocks for replication tests
- `dind/` - Docker-in-Docker configuration for nested container tests

### CI/CD Integration

Tests are automatically run via GitHub Actions when you create a pull request:

- `.github/workflows/test.yml` - Main test workflow (runs automatically on PR)
- `.github/workflows/clt_tests.yml` - CLT tests (runs automatically on PR)
- `.github/workflows/clt_nightly.yml` - Extended nightly tests

**For contributors**: Simply add your tests to the appropriate directory in `test/clt-tests/`. When you create a PR, GitHub Actions will automatically run all relevant tests. Maintainers will determine if additional test coverage (e.g., nightly tests) is needed during code review.

**You don't need to worry about which workflow runs your tests** - the CI/CD system handles this automatically based on your changes.

### Binary and File Locations

After building Manticore locally:

**Linux:**
- Binary: `build/src/searchd`
- Config: `/etc/manticoresearch/manticore.conf`
- Logs: `/var/log/manticore/searchd.log`
- Data: `/var/lib/manticore/`

**macOS:**
- Binary: `build/src/searchd`
- Config: `/usr/local/etc/manticoresearch/manticore.conf`
- Logs: `/usr/local/var/log/manticore/searchd.log`
- Data: `/usr/local/var/lib/manticore/`

**Docker:**
- Config: `/etc/manticoresearch/manticore.conf`
- Logs: `/var/log/manticore/searchd.log`
- Data: `/var/lib/manticore/`

## Contributing

When submitting a pull request:

1. Ensure all existing tests pass
2. Add tests for new functionality
3. Update tests if changing existing behavior
4. Tests will run automatically via GitHub Actions
5. Address any test failures before requesting review

For more details, see [CONTRIBUTING.md](CONTRIBUTING.md).
