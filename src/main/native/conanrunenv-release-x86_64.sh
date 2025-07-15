script_folder="/Users/stevenmeyer/Projects/intervox_maven/Intervox/src/main/native"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in VK_DRIVER_FILES VK_ICD_FILENAMES PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH
do
    is_defined="true"
    value=$(printenv $v) || is_defined="" || true
    if [ -n "$value" ] || [ -n "$is_defined" ]
    then
        echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
    else
        echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
    fi
done


export VK_DRIVER_FILES="/Users/stevenmeyer/.conan2/p/b/molte3d22cbeef2742/p/lib/MoltenVK_icd.json:$VK_DRIVER_FILES"
export VK_ICD_FILENAMES="/Users/stevenmeyer/.conan2/p/b/molte3d22cbeef2742/p/lib/MoltenVK_icd.json:$VK_ICD_FILENAMES"
export PATH="/Users/stevenmeyer/.conan2/p/b/molte3d22cbeef2742/p/bin:$PATH"
export LD_LIBRARY_PATH="/Users/stevenmeyer/.conan2/p/b/molte3d22cbeef2742/p/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="/Users/stevenmeyer/.conan2/p/b/molte3d22cbeef2742/p/lib:$DYLD_LIBRARY_PATH"