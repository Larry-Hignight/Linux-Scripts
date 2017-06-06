# Set git to use the credential memory cache
git config --global credential.helper cache

# Set the cache to timeout after a very long time
git config --global credential.helper 'cache --timeout=99999999999'
