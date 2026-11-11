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

"${objcopy}" -O binary -j .text.secure_vector "${inputfilename}" "${filebody}_secure_vector.bin"
"${objcopy}" -O binary -j .text.non_secure_vector "${inputfilename}" "${filebody}_non_secure_vector.bin"
"${objcopy}" -O binary -j .text.secure_code "${inputfilename}" "${filebody}_secure_code.bin"
"${objcopy}" -O binary -j .text.non_secure_code -j .data "${inputfilename}" "${filebody}_non_secure_code.bin"

echo "Post build script complete"
