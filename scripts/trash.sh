#!/bin/bash

declare TRASH_DIR=$HOME/.trash
declare TRASH_LOG=$TRASH_DIR/.log
declare ID_LIST=""

record_delete()
{
    NAME=$1
    SRC=$2
    DST=$3
    INDEX=1
    if [ -f $TRASH_LOG ]; then
	MAX_IDX=`awk -F',' 'BEGIN{IDX=0}{if($1 > IDX) IDX=$1;}END{print IDX}' $TRASH_LOG`
	INDEX=$((MAX_IDX+1))
    fi
    echo "$INDEX,$NAME,$SRC,$DST" >> $TRASH_LOG
}

do_delete()
{
    ITEMS="$@"
    mkdir -p $TRASH_DIR
    for I in $ITEMS; do
	i=`echo $I | grep -oe '^[^-]\+\.*'`
	if [[ -n $i ]]; then
	    SRC_PATH=`readlink -f $i`
	    BASENAME=`basename $SRC_PATH`
	    DST_PATH=`mktemp -u -p $TRASH_DIR ${BASENAME}.XXXXXX`
	    mv $SRC_PATH $DST_PATH
	    record_delete $BASENAME $SRC_PATH $DST_PATH
	    echo "Move '$SRC_PATH' to Trash"
	fi
    done
}

ls_delete()
{
    if [ -f $TRASH_LOG ]; then
	EVAL_S=`awk -F',' 'BEGIN{L1=2;L2=4;L3=4}/^[^[:space:]]+/{if(length($1)>L1) L1=length($1); if(length($2)>L2) L2=length($2); if(length($3)>L3) L3=length($3);}END{printf("ID_LEN=%d;NAME_LEN=%d;PATH_LEN=%d;",L1,L2,L3)}' $TRASH_LOG`
	eval $EVAL_S
	echo "id,name,path" | awk -F',' -v L1=$ID_LEN -v L2=$NAME_LEN -v L3=$PATH_LEN 'BEGIN{FMT="| %-"L1"s | %-"L2"s | %-"L3"s |\n"}{printf(FMT,$1,$2,$3)}'
	echo -n '+'
	for LEN in $ID_LEN $NAME_LEN $PATH_LEN; do
	    echo -n '-'
	    for i in `seq $LEN`; do
		echo -n '-'
	    done
	    echo -n '-+'
	done
	echo ''
	awk -F',' -v L1=$ID_LEN -v L2=$NAME_LEN -v L3=$PATH_LEN 'BEGIN{FMT="| %-"L1"s | %-"L2"s | %-"L3"s |\n"}/^[^[:space:]]+/{printf(FMT,$1,$2,$3)}' $TRASH_LOG
    else
	echo -e "| id | name | path |"
	echo -e "+----+------+------+"
    fi
}

get_ids()
{
    ARGS="$@"
    ID_L=0; ID_H=0; ID_N=0;
    for i in $ARGS; do
	local EVAL_S=`echo $i | awk -F '[,-]' '{printf("ID_N=%d;ID_L=%d;ID_H=%d;",NF,$1,$2)}'`
	eval $EVAL_S
	if [[ $ID_N -eq 1 && $ID_L -gt 0 ]]; then
	    ID_LIST="$ID_LIST $ID_L"
	elif [[ $ID_N -eq 2 && $ID_L -gt 0 && $ID_H -gt 0 && $ID_H -gt $ID_L ]]; then
	    ID_LIST="$ID_LIST `seq $ID_L $ID_H`"
	else
	    echo "IllegalArguments: $i"
	    continue
	fi
    done
    ID_LIST=`echo $ID_LIST | tr ' ' '\n' | sort | uniq`
}

undo_delete()
{
    if [ -f $TRASH_LOG ]; then
	get_ids "$@"
	for ID_I in $ID_LIST; do
	    CMD=`grep -e "^${ID_I}," $TRASH_LOG | awk -F',' '/^[^[:space:]]+/{if($3 != "" && $4 != "") printf("mv %s %s",$4,$3)}'`
	    [[ -n $CMD ]] && eval $CMD && sed -i -e "/^${ID_I},/d" $TRASH_LOG
	    #[[ -n $CMD ]] && echo $CMD
	done
    fi
}

redo_delete()
{
    if [ -f $TRASH_LOG ]; then
	get_ids "$@"
	for ID_I in $ID_LIST; do
	    CMD=`grep -e "^${ID_I}," $TRASH_LOG | awk -F',' '/^[^[:space:]]+/{if ($4 != "" && $4 != "/" && $4 != "~") printf("/usr/bin/rm -rf %s",$4);}'`
	    [[ -n $CMD ]] && eval $CMD && sed -i -e "/^${ID_I},/d" $TRASH_LOG
	    #[[ -n $CMD ]] && echo $CMD
	done
    fi
}

alias rm="do_delete"
alias lsrm="ls_delete"
alias unrm="undo_delete"
alias rfrm="redo_delete"
