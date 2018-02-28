#!/bin/sh
set -e
echo "Welcome to home page, a simple webpage generator."
echo
if [ "$1" ]
then
  FILENAME="$1"
else
  echo "Enter the filename for this page. If it's the main page then it should be"
  echo "named 'index.htm.' Subsequent pages can be named anything else."
  read FILENAME
fi

if [ -z "${FILENAME}" ]
then
  echo "No filename given. Exiting." 1>&2
  exit 1
fi


if [ -e "${FILENAME}" ]
then
  echo "${FILENAME} already exists. Overwrite? (y/N)"
  read OVERWRITE || exit
  if [ "${OVERWRITE}" '!=' "y" ]
  then
    echo "Not overwriting." 1>&2
    exit
  fi
fi

echo "Enter the title you are going to use for this page. It will appear at"
echo 'the top of each page and identifies the page. Example: "My Home Page"'
read TITLE
#clear

# put some header info in the file
echo "<html>" > "${FILENAME}"
echo "<head>" >> "${FILENAME}"
echo "<title>${TITLE}</title>" >> "${FILENAME}"
echo "</head>" >> "${FILENAME}"
echo "<body>" >> "${FILENAME}"


echo "Begin entering text below this line."
echo "Press enter on a blank line for the options menu."
DONE=false
until $DONE
do
  read LINE || DONE=true
  if [ -z "${LINE}" ]
  then
    # show the menu
    READ_MORE_OPTIONS=true
    while $READ_MORE_OPTIONS
    do
      READ_MORE_OPTIONS=false
      echo "<c>enter text  <l>ink reference  <m>ail tag  <p>aragraph"
      echo "<q>uit  <r>eturn to text entry"
      read OPTION
      case $OPTION in
        c|C)
          # center text
          echo "Enter line of text to be centered"
          read LINE && echo "<center>${LINE}</center>" >> "${FILENAME}"
        ;;

        l|L)
          # add a link
          echo "Enter link reference here,"
          echo "usually to another page or place on the web"
          echo "or to a picture or sound file(example: page2.htm)"
          read URL && if [ "${URL}" ]
          then
            echo "Enter link description here"
            echo "(example: Click here for XXX)"
            read LINE && if [ "${LINE}" ]
            then
              echo "<p><a href='${URL}'>${LINE}</a>" >> "${FILENAME}"
            fi
          fi
        ;;

        m|M)
          # add a mail reference
          echo "Enter e-mail address to be used"
          read EMAIL && if [ "${EMAIL}" ]
          then
            echo "Enter mail message (example: Click here to email me)"
            read LINE && if [ "${LINE}" ]
            then
              echo "<p><a href='mailto:${EMAIL}'>${LINE}</a>" >> "${FILENAME}"
            fi
          fi
        ;;

        p|P)
          # start a new paragraph
          echo "<p>" >> "${FILENAME}"
        ;;

        q|Q)
          # close the tags and quit
          echo "</body>" >> "${FILENAME}"
          echo "</html>" >> "${FILENAME}"
          DONE=true
        ;;

        r|R)
          # stop reading more options
        ;;
        *)
          READ_MORE_OPTIONS=true
        ;;

      esac
    done
  else
    # append the line to the output file
    cat >> "${FILENAME}" <<EOF
${LINE}
EOF
  fi
done
