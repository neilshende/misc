#!/bin/bash

function dircontains_syntax {
    local msg=$1
    echo "${msg}" >&2
    echo "syntax: dircontains <parent> <file>" >&2
    return 1
}

function dircontains {
    local result=1
    local parent=""
    local parent_pwd=""
    local child=""
    local child_dir=""
    local child_pwd=""
    local curdir="$(pwd)"
    local v_aux=""

    # parameters checking
    if [ $# -ne 2 ]; then
        dircontains_syntax "exactly 2 parameters required"
        return 2
    fi
    parent="${1}"
    child="${2}"

    # exchange to absolute path
    parent="$(readlink -f "${parent}")"
    child="$(readlink -f "${child}")"
    dir_child="${child}"

    # direcory checking
    if [ ! -d "${parent}" ];  then
        dircontains_syntax "parent dir ${parent} not a directory or doesn't exist"
        return 2
    elif [ ! -e "${child}" ];  then
        dircontains_syntax "file ${child} not found"
        return 2
    elif [ ! -d "${child}" ];  then
        dir_child=`dirname "${child}"`
    fi

    # get directories from $(pwd)
    cd "${parent}"
    parent_pwd="$(pwd)/"
    cd "${curdir}"  # to avoid errors due relative paths
    cd "${dir_child}"
    child_pwd="$(pwd)/"

    # checking if is parent
    [ "${child_pwd:0:${#parent_pwd}}" = "${parent_pwd}" ] && result=0

    # return to current directory
    cd "${curdir}"
    return $result
}    
