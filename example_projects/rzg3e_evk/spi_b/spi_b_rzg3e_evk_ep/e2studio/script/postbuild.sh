echo "Post build script started"

if [ -n "$1" ]; then
  inputfilename=$1
elif [ -n "$BuildArtifactFileName" ]; then
  inputfilename=$BuildArtifactFileName
elif [ -n "$BUILDARTIFACTFILENAME" ]; then
  inputfilename=$BUILDARTIFACTFILENAME
fi

if [ -n "$2" ]; then
  objcopy=$2
else
  objcopy=arm-none-eabi-objcopy
fi

filebody=${inputfilename%.*}

"${objcopy}" -O binary -R .resource_table "${filebody}.elf" "${filebody}.bin"
"${objcopy}" -I binary -O srec --adjust-vma 0x60000000 --srec-forceS3 "${filebody}.bin" "${filebody}.srec"

echo "Post build script complete"
