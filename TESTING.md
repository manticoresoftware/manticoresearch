# Testing

This is a comprehensive guide to the automated test suites included in Manticore Search. Testing ensures that new changes are stable, free of bugs, and don't introduce regressions. This document covers both traditional CTest-based tests and modern CLT (Command Line Testing) tests.

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

Manticore Search has two main test frameworks:

1. **CTest Tests** - Traditional C++ unit and regression tests run via CMake's CTest
2. **CLT Tests** - Modern command-line tests in `test/clt-tests/` using the [CLT framework](https://github.com/manticoresoftware/clt)

## Test Types

### CTest Tests (Legacy)
Located in `test/test_XXX/` directories. These tests verify core C++ functionality and are run via CMake/CTest.

### CLT Tests (Modern Approach)
Located in `test/clt-tests/` and organized by functionality:

- **base/** - Reusable blocks for starting/stopping searchd
- **buddy/** - Buddy plugin system tests
- **buddy-plugins/** - Tests for Buddy plugins (fuzzy search, autocomplete, etc.)
- **bugs/** - Regression tests for specific bug fixes
- **core/** - Core searchd functionality tests
- **data-manipulation/** - DDL/DML operations (ALTER, INSERT, REPLACE, etc.)
- **expected-errors/** - Negative tests validating error handling
- **fulltext-search/** - Full-text search functionality (MATCH, ranking, etc.)
- **http-interface/** - HTTP API tests
- **indexer/** - Indexer and data import tests
- **indexing-error/** - Tests for indexing error conditions
- **installation/** - Package installation and upgrade tests
- **integrations/** - Integration tests with external services (Kafka, etc.)
- **join/** - JOIN operations tests
- **kibana/** - Kibana integration tests
- **mcl/** - Manticore Columnar Library tests
- **migration-es-ms/** - Elasticsearch to Manticore migration tests
- **mysqldump/** - MySQL dump import/export tests
- **opensearch/** - OpenSearch compatibility tests
- **performance/** - Performance benchmarks
- **performance-nightly/** - Extended nightly performance tests
- **replication/** - Replication and clustering tests
- **scripts/** - Helper scripts for test data generation
- **sharding/** - Distributed table and sharding tests
- **tables-interaction/** - Tests for table-to-table interactions
- **test-configuration/** - Configuration and settings tests
- **vector-knn/** - Vector/KNN search tests

## Testing via GitHub Actions

**Recommended for most contributors**: Instead of setting up a local test environment, you can use GitHub Actions to run tests automatically.

### How it works:

1. **Fork the repository** on GitHub
2. **Create a branch** in your fork
3. **Make your changes** and commit them
4. **Push to your fork** - This automatically triggers GitHub Actions
5. **Create a Pull Request** - Tests run automatically on PR creation and updates

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

For local CLT test execution, you need Docker and the CLT action. Tests run inside Docker containers to ensure consistent environment.

**Using GitHub CLI and Act (for local GitHub Actions simulation):**

```bash
# Install act (GitHub Actions local runner)
brew install act  # macOS
# or
curl -s https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash  # Linux

# Run specific CLT test suite locally
act -j clt --matrix test-suite:name:Buddy-plugins
```

**Direct Docker execution (manual approach):**

```bash
# Build or pull Manticore image
docker pull manticoresearch/manticore:latest

# Run test manually in container
docker run --rm -v $(pwd):/manticore manticoresearch/manticore:latest bash -c "
  cd /manticore &&
  mysql -h127.0.0.1 -P9306 -e 'SHOW TABLES;'
"
```

**Note**: The CLT framework is integrated as a GitHub Action (`manticoresoftware/clt@0.7.3`), not as a standalone CLI tool. For full CLT capabilities, use GitHub Actions or the Act tool for local simulation.

### CLT Test File Format

CLT tests use `.rec` (test case) and `.recb` (reusable block) files:

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

See existing tests in `test/clt-tests/` for examples.

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

Tests are automatically run via GitHub Actions:

- `.github/workflows/test.yml` - Main test workflow
- `.github/workflows/clt_tests.yml` - CLT test workflow  
- `.github/workflows/clt_nightly.yml` - Nightly extended tests

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

