# The following script transforms the output from the w command across chime and bell
# On chime, only the output from remotely logged in user data is displayed
# Since bell cannot be accessed outside the local network, all user login data is displayed

library(stringr)

display_host_info <- function(x, hostname) {
  cat(str_c('Host: ', hostname, ' - ', x[1]), sep = '\n')
  if (length(x) > 2) cat(x[2], sep = '\n')
}

# Capture the output from the w command
chime <- system2('w', stdout=TRUE)
bell <- system2(command = 'ssh', args = c('chime@192.168.0.2', 'w'), stdout=TRUE)

# Display the output
display_host_info(chime, 'Chime')
if (length(chime) > 2) {
  for (line in chime[-1:-2]) if (str_detect(line, 'pts/[0-9]+')) cat(line, sep = '\n')
} else cat('No remote users', sep = '\n')
cat('\n')
display_host_info(bell, 'Bell')
if (length(bell) > 2) {
  for (line in bell[-1:-2]) cat(line, sep = '\n')
} else cat('No remote users', sep = '\n')
cat('\n')
