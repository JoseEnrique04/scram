#!usr/bin/env python

import os
from subprocess import call

from nose.tools import assert_true, assert_equal, assert_not_equal

def test_fta_calls():
    """Tests all possible calls from the cli."""

    # Correct inputs
    fta_input = "./input/fta/correct_tree_input_with_probs.xml"
    fta_no_prob = "./input/fta/correct_tree_input.xml"

    # Test help
    cmd = ["scram", "--help"]
    yield assert_equal, 0, call(cmd)

    # Test version information
    cmd = ["scram", "--version"]
    yield assert_equal, 0, call(cmd)

    # Empty call
    cmd = ["scram"]
    yield assert_equal, 1, call(cmd)

    # Test the validation a fta tree file
    cmd = ["scram", "-v", fta_input]
    yield assert_equal, 0, call(cmd)

    # Test the incorrect limit order
    cmd = ["scram", fta_input, "-l", "-1"]
    yield assert_not_equal, 0, call(cmd)

    # Invalid argument type for an option
    cmd = ["scram", fta_input, "-l", "string_for_int"]
    yield assert_equal, 1, call(cmd)

    # Test the limit order no minimal cut sets.
    # This was an issue #17. This should not throw an error anymore.
    cmd = ["scram", fta_input, "-l", "1"]
    yield assert_equal, 0, call(cmd)

    # Test the incorrect cut-off probability
    cmd = ["scram", fta_input, "-c", "-1"]
    yield assert_not_equal, 0, call(cmd)
    cmd = ["scram", fta_input, "-c", "10"]
    yield assert_not_equal, 0, call(cmd)

    # Test the incorrect number for probability series
    cmd = ["scram", fta_input, "-s", "-1"]
    yield assert_not_equal, 0, call(cmd)

    # Test the application of the rare event and MCUB at the same time
    cmd = ["scram", fta_input, "--rare-event", "--mcub"]
    yield assert_not_equal, 0, call(cmd)

    # Test graph only
    cmd = ["scram", fta_input, "-g"]
    yield assert_equal, 0, call(cmd)
    graph_file = "./input/fta/TwoTrains.dot"
    cmd = ["scram", fta_input, "-g", "-o", graph_file]
    yield assert_equal, 0, call(cmd)
    # Test if output is created
    yield assert_true, os.path.isfile(graph_file)
    # Changing permission
    cmd = ["chmod", "a-w", graph_file]
    call(cmd)
    cmd = ["scram", fta_input, "-g", "-o", graph_file]
    yield assert_not_equal, 0, call(cmd)
    if os.path.isfile(graph_file):
        os.remove(graph_file)

    # Test calculation calls
    cmd = ["scram", fta_no_prob]
    yield assert_equal, 0, call(cmd)
    out_temp = "./output_temp.xml"
    cmd.append("-o")
    cmd.append(out_temp)
    yield assert_equal, 0, call(cmd)  # Report into an output file
    if os.path.isfile(out_temp):
        os.remove(out_temp)

    # Test with a configuration file
    config_file = "./input/fta/full_configuration.xml"
    cmd = ["scram", "--config-file", config_file, "-o", out_temp]
    yield assert_equal, 0, call(cmd)
    if os.path.isfile(out_temp):
        os.remove(out_temp)

    # Test the clash of files from configuration and command-line
    config_file = "./input/fta/full_configuration.xml"
    cmd = ["scram", "--config-file", config_file,
            "input/fta/correct_tree_input_with_probs.xml"]
    yield assert_not_equal, 0, call(cmd)

    # Test the rare event approximation
    cmd = ["scram", fta_input, "--rare-event"]
    yield assert_equal, 0, call(cmd)

    # Test the MCUB approximation
    cmd = ["scram", fta_input, "--mcub"]
    yield assert_equal, 0, call(cmd)

    # Run with logging
    cmd = ["scram", fta_input, "--log"]
    yield assert_equal, 0, call(cmd)