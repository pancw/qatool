#!/bin/bash

#pgrep y5$1 | xargs kill -2    
#pgrep y5fight$1 | xargs kill -2    
#pgrep y5gate$1 | xargs kill -2
#pgrep y5agent$1 | xargs kill -2
#pgrep y5backer$1 | xargs kill -2

pgrep y2$1 | xargs kill -2
pgrep y2gate$1 | xargs kill -2
pgrep y2agent$1 | xargs kill -2
pgrep y2backer$1 | xargs kill -2

