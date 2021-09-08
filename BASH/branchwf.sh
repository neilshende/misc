function gcreateb() {
    if [ $# -ne 2 ] ;then
     echo usage : $0 repo branch
     return
  fi
  cd ~/work/$1
  git checkout -b $2
  echo Now you can make changes 
  echo remember to "git add ."
  echo             "git commit -m message"
}
function gpushb() {
  if [ $# -ne 2 ] ;then
     echo usage : $0 branch
     return
  fi
  git push -u origin $1
}
function gmergeb() {
  if [ $# -ne 2 ] ;then
     echo usage : $0 branch
     return
  fi
  git fetch upstream
  git checkout $1
  git rebase -i upstream/master
  git push --force
  echo now create a pull request on github.
  echo update the review comments using "git add files"
  echo                                  "git commit --ammend"
  echo              followed by         $0 $1
  echo
  echo on "ship it" commit on github.
}

