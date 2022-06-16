#! bin/bash

CFG_PATH="./.github/integration_test.cfg"
LOG_PATH="./it/log.txt"

line_num=0

# Logging Purposes
mkdir -p ./it
touch $LOG_PATH

# bazel clean

# Integration Tests
while IFS= read -r integration_test
do
    ((line_num++))
    # TODO Better Error Checking
    if [ -z "$integration_test" ]; then
        printf "EMPTY INPUT ON LINE %d\n\n" "$line_num" >> $LOG_PATH
    elif [[ $integration_test == *"bazel build"* ]]; then
        # Logging Purposes
        if [ -s $LOG_PATH ]; then 
            printf "\n" >> $LOG_PATH; 
        fi
        printf "${integration_test#*:}\n\n" >> $LOG_PATH

        # Building
        ${integration_test#*=}
    elif [[ $integration_test == *"output_dir="* ]]; then
        # Output Directories
        output_dir_expected=${integration_test#*output_dir=}
        output_dir_expected=${output_dir_expected% *}
        mkdir -p $output_dir_expected
        
        replace_dir="output"
        output_dir_output="${output_dir_expected/expected/"$replace_dir"}"
        mkdir -p $output_dir_output

        # Run Integration Tests
        ${integration_test#*=}
        integration_test="${integration_test/expected/"$replace_dir"}"
        ${integration_test#*=}

        # Compare Outputs & Logging Purposes
        DIFF=$(diff -qr $output_dir_expected $output_dir_output)
        if [ "$DIFF" ]; then 
            echo "$DIFF" >> $LOG_PATH
        else 
            printf "No Diff" >> $LOG_PATH
        fi
    else 
        printf "INVALID INPUT ON LINE %d\n\n" "$line_num" >> $LOG_PATH
    fi
done < $CFG_PATH
