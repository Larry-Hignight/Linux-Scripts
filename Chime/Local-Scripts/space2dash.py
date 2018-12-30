import os

def remove_prefix(s, prefix):
    return s[len(prefix):] if s.startswith(prefix) else s




files = [f for f in os.listdir(os.getcwd()) if os.path.isfile(f)]
for f in files:
    #print f
    old_file = os.path.join(os.getcwd(), f)
    end=f.replace(" ", "-").replace("(","").replace(")","").lower().replace("&","-and-").replace("[","").replace("]","").replace("`","").replace("'","").replace("~","").lower()
    while end.startswith("-"):
        end = remove_prefix(end, "-")
    new_file = os.path.join(os.getcwd(), end)
    os.rename(old_file, new_file)
