echo "==== Setting the globel user.name and user.email ===="
git config --global user.name "Larry-Hignight"
git config --global user.email "larry.hignight@gmail.com"

echo "==== Setting git to use the credential memory cache ===="
git config --global credential.helper cache

echo "==== Setting the cache to timeout after a very long time ===="
git config --global credential.helper 'cache --timeout=9999999999999'
