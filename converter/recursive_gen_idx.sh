#!/bin/bash


NO_BGZIP_MSG="was bgzip used to compress this file"
UNSORTED_CHR_MSG="the chromosome blocks not continuous"
UNSORTED_BAM_MSG="the alignment is not sorted"

MID="SRT"

# bed type files


if [ "$#" -lt 1 ]
then
  echo "Recursivley generate index of data file to visualize it in WashU Epigenome Browser."
  echo "If data files is not .gz or unsorted, sort and then compress it with bgzip."
  echo ""
  echo "Usage: recursive_gen_idx.sh [DIR] [EXT](optional)"
  echo ""
  echo '[EXT] should one of ("bam" "tagalign" "bed" "bedpe" "bedgraph" "bg" "bdg" "narrowpeak" "broadpeak" "gappedpeak")'
  echo ""
  exit 1
fi

if [ ! -d $1 ]
then
  echo "Directory doesn't exists!"
  exit 2
fi

EXTS=("bam" "tagAlign" "tagalign" "bed" "bedpe" "bedGraph" "bedgraph" "bg" "bdg" "narrowPeak" "narrowpeak" "broadPeak" "broadpeak" "gappedPeak" "gappedpeak")

if [ $# -eq 2 ];
then
  EXTS=("$2")
fi

shopt -s nullglob

CURR_DIR=$PWD

# add script directory to PATH (due to python files in it)
cd $(dirname $0)
SH_PATH=$(pwd -P)
#echo "Script directory: ${SH_PATH}"
P2H_PATH=${SH_PATH}/peak2hammock

# go to the working directory
cd ${CURR_DIR}
cd $1

# iterate on file extensions
for EXT in "${EXTS[@]}"
do

  # first sort and bgzip uncompressed files, look for bam too
  for FILE in $(find . -type f -name "*.$EXT" ! -name "*.SRT.*" ); do

    if [[ $EXT == "bam" ]]; then
      if [ ! -f  $FILE.bai ]; then        
        MSG=$(samtools index $FILE 2>&1)

        if [[ $MSG == *${UNSORTED_BAM_MSG}* ]]; then
          echo "sorting and making index of $FILE... (reason: $MSG)"
          DIR2=$(dirname $FILE)
          FILE2=$DIR2/$(basename $FILE .${EXT}).$MID.${EXT}
          samtools sort $FILE -f $FILE2
          samtools index $FILE2
        else
          echo "making index (.bai) of $FILE..."
        fi
      fi
    else
      if [ ! -f  $FILE.gz ]; then
        echo bgzipping $FILE...
        cat $FILE | sed '/^\(chr\)/!d' | sort -k1,1V -k2,2n | bgzip -f -c > $FILE.gz
      fi
    fi

  done

  # loop over compressed files and generate index using tabix
  for FILE in $(find . -type f -name "*.$EXT.gz" ! -name "*.SRT.*" ); do
    case "$EXT" in

    "narrowPeak" | "gappedPeak" | "broadPeak" | "narrowpeak" | "gappedpeak" | "broadpeak")
      echo converting $FILE to hammock...

      P2H=${P2H_PATH}/${EXT,,}.py
      FILE2=${FILE%.gz}.hammock
      rm -f $FILE2.gz $FILE2.gz.tbi
      TMP=$FILE2.tmp
      gzip -d -c $FILE | sed '/^\(chr\)/!d' | sort -k1,1V -k2,2n > $TMP
      python $P2H $TMP $FILE2
      rm -r $FILE2.tmp
      ;;

    *)
      MSG=$(tabix -p bed $FILE 2>&1)
      if [[ $MSG == *${NO_BGZIP_MSG}* ]] || [[  $MSG == *${UNSORTED_CHR_MSG}* ]]; then
        echo "sorting, bgzipping and making index of $FILE... (reason: $MSG)"
        DIR2=$(dirname $FILE)
        FILE2=$DIR2/$(basename $FILE .${EXT}.gz).$MID.${EXT}.gz
        gzip -d -c $FILE | sed '/^\(chr\)/!d' | sort -k1,1V -k2,2n | bgzip -f -c > $FILE2
        FILE2=$FILE2
        tabix -p bed $FILE2
      else
        echo "making index (.tbi) of $FILE..."
      fi
      ;;

    esac

  done
done

