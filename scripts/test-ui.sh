#!/usr/bin/bash
pytest -v --tb=short --device nanos $@
pytest -v --tb=short --device nanosp $@
pytest -v --tb=short --device nanox $@
pytest -v --tb=short --device stax $@
