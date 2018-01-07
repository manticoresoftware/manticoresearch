# Contributing to Manticore Search

We're happy you want to contribute!
This project adheres to the Manticore Search [Code of Conduct](https://github.com/manticoresoftware/manticore/blob/master/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. 
There are many ways to cotribute, from helping others, spread the word, submitting bug reports and feature requests or writing code.


## Reporting bugs

No software is perfect, so bugs can happen. 
Before submitting a bug make sure to :

* check if the bug is not already reported at  [issues list](https://github.com/manticoresoftware/manticore/issues) 
* the bug reproduces on the latest version. If you are using an older version, it is possible that your bug may be fixed already. 
 

In case the bug is already reported, you can help by participate in the discussion by confirming you are affected as well and check if you can provide additional information about the bug.

To make things easier and fix it faster, try to provide a small test case which can be run to confirm your bug. The maintainers needs to be able to reproduce the bug in order to fix it.

If you can provide sample data, but it's big, we have a [Write-only FTP](https://github.com/manticoresoftware/manticore/wiki/Write-only-FTP) for uploading larger data on one of our servers.

Follow the [issue template](https://github.com/manticoresoftware/manticore/blob/master/ISSUE_TEMPLATE.md) guideline about the information the bug report should include.


## Feature requests

A lot of features in Manticore Search come from user's requests. To make a feature request, either open an issue on our [issues list](https://github.com/manticoresoftware/manticore/issues) on Github or on [Feature Requests](https://forum.manticoresearch.com/c/feature-requests) forum section. Describe in detail the feature you would like to see, which are it's use cases and how it should work.

## Code changes

We recommend opening first a github issue describing your proposed changed and check if they fit with what maintainers are doing and have planned. 

### Fork/clone the repository
Clone/fork the master branch via Github or 'git clone'.
Don't work directly on the master branch, but create a branch.

### Testing and submiting changes

Manticore Search code comes with a test suite which must be run to ensure your changes don't create any regression. Read the [TESTING](https://github.com/manticoresoftware/manticore/blob/master/TESTING.md) guide for how to run the tests.

We recommend to update your local repository with the latest code from the official repository and rebase the branch on top of the latest master branch.
If later changes are needed, you can add them as separate commits. 

If you are fixing a regression, add/update a functional test.

The commit message should contain a short summary of changes. If there is an open issue about the changes you are making, please include in the commit message the issue number.

When everything is ready, submit a pull request.  The pull request title should sum up the changes made and in the body provide more details about what your changes do.
Be patient, it my take a while until your pull request gets accepted. 

