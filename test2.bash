#!/bin/bash


branches=`git branch --all | grep -E '^\s*remotes/origin/manticore-[0-9]+'`
for branch in ${branches[@]}; do
  #git fetch origin branch
  cur_branch=$(echo "$branch" |  sed "s#remotes/origin/##") 
  if  [[ "$cur_branch" == "manticore-10.1.0" || "$cur_branch" == "manticore-13.11.0" || "$cur_branch" == "manticore-13.11.1" ]]; then
    continue
  fi
  echo $cur_branch
  git checkout $cur_branch
  updated=0
  if [ -f ".github/workflows/deploy_docs.yml" ]; then
    res=`cat '.github/workflows/deploy_docs.yml' | sed 's#        CACHEB: "../cache"#        GH_ACCESS_TOKEN: ${{ secrets.GH_ACCESS_TOKEN }}\n        CACHEB: "../cache"#g'`
    echo "$res" > .github/workflows/deploy_docs.yml
    git add .github/workflows/deploy_docs.yml
    updated=1
  fi
  if [ -f ".github/workflows/check_docs.yml" ]; then
    res=`cat '.github/workflows/check_docs.yml' | sed 's#        CACHEB: "../cache"#        GH_ACCESS_TOKEN: ${{ secrets.GH_ACCESS_TOKEN }}\n        CACHEB: "../cache"#g'`
    echo "$res" > .github/workflows/check_docs.yml
    git add .github/workflows/check_docs.yml
    updated=1
  fi
  if [[ "$updated" == "1" ]]; then
    echo "updated"
    git commit -m 'Updated deploy job environment'
    sudo git push origin $cur_branch
  fi
done

