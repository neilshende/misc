#on github create a fork for VivekMemverge for the repos mvmalloc and criu
#on dev vm:
cd ~/work
git clone github.org:VivekMemverge/mvmalloc.git
git clone github.org:VivekMemverge/criu.git

function gsetup() {
  if [ $# -ne 1 ] ;then
  echo usage : $0 repo
  return
  fi
  cd ~/work/${1}
  git remote add upstream git@github.com:memverge/${1}.git
  git remote set-url --push upstream no_push
  git remote -v
}

#wf for working on mvmalloc master
#note that master for criu is mvcriu
function wf1() {
  if [ $# -ne 1 ] ;then
  echo usage : $0 message
  return
  fi
  git add .
  git commit -m "${1}"
  git push
}
function wf2() {
  git fetch upstream
  git checkout master
  git rebase upstream/master
  git push --force
  echo "now go to github and create a pull request."
}
function wf5() {
  git fetch upstream
  git checkout master
  git rebase -i upstream/master
  git push --force
  echo "now submit the change to upstream from github."
}
function wf7() {
  git fetch upstream
  git checkout master
  git rebase upstream/master
  git push --force
}

#Work flow for creating feature/bug branch.
function gcreateb() {
  if [ $# -ne 2 ] ;then
  echo usage : $0 repo branch
  return
  fi
  cd ~/work/$1
  git checkout -b $2
  echo Now you can make changes
  echo remember to \"git add .\"
  echo \"git commit -m message\"
}
function gpushb() {
  if [ $# -ne 1 ] ;then
  echo usage : $0 branch
  return
  fi
  git push -u origin $1
}
function gmergeb() {
  if [ $# -ne 1 ] ;then
  echo usage : $0 branch
  return
  fi
  git fetch upstream
  git checkout $1
  git rebase -i upstream/master
  git push --force
  echo now create a pull request on github.
  echo update the review comments using \"git add files\"
  echo \"git commit\"
  echo followed by $0 $1
  echo
  echo on \"ship it\", \"Squash and merge\" pull request then commit on github.
}

