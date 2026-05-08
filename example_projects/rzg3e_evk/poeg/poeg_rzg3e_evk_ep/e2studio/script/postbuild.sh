echo "Post build script started"

if [ -n "$BuildArtifactFileName" ]; then
  inputfilename=$BuildArtifactFileName
fi
if [ -n "$BUILDARTIFACTFILENAME" ]; then
  inputfilename=$BUILDARTIFACTFILENAME
fi

filebody=${inputfilename%.*}

arm-none-eabi-objcopy -O binary -R .resource_table ${filebody}.elf ${filebody}.bin
arm-none-eabi-objcopy -I binary -O srec --adjust-vma 0x60000000 --srec-forceS3 ${filebody}.bin ${filebody}.srec

echo "Post build script complete"
