#!/bin/bash

TRASH_DIR=$HOME/.trash
TRASH_LOG=$TRASH_DIR/.log

record()
{
    NAME=$1
    SRC=$2
    DST=$3
    if [ -f $TRASH_LOG ]; then
	INDEX=`tail -1 $TRASH_LOG | awk -F ',' '{if ($1 > 0) IDX=$1 + 1; else IDX=1; print IDX;}'`
    else
	touch $TRASH_LOG
	INDEX=1
    fi
    echo "$INDEX,$NAME,$SRC,$DST" >> $TRASH_LOG
}

trash()
{
    ITEMS="$@"
    mkdir -p $TRASH_DIR
    for i in $ITEMS; do
	SRC_PATH=`readlink -f $i`
	BASENAME=`basename $SRC_PATH`
	DST_PATH=`mktemp -u -p $TRASH_DIR ${BASENAME}.XXXXXX`
	mv $SRC_PATH $DST_PATH
	record $BASENAME $SRC_PATH $DST_PATH
    done
}

lstrash()
{
    if [ -f $TRASH_LOG ]; then
	EVAL_S=`awk -F',' 'BEGIN{L1=2;L2=4;L3=4}{if(length($1)>L1) L1=length($1); if(length($2)>L2) L2=length($2); if(length($3)>L3) L3=length($3);}END{printf("ID_LEN=%d;NAME_LEN=%d;PATH_LEN=%d;",L1,L2,L3)}' $TRASH_LOG`
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
	awk -F',' -v L1=$ID_LEN -v L2=$NAME_LEN -v L3=$PATH_LEN 'BEGIN{FMT="| %-"L1"s | %-"L2"s | %-"L3"s |\n"}{printf(FMT,$1,$2,$3)}' $TRASH_LOG
    else
	echo -e "| id | name | path |"
	echo -e "+----+------+------+"
    fi
}

revert()
{
    ARGS="$@"
    for i in $ARGS; do
	echo $i | awk -F '[,-]' '{}'
}
