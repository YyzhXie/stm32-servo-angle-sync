# GitHub 工作流

本项目已经关联 GitHub 仓库：

```text
https://github.com/YyzhXie/stm32-servo-angle-sync
```

## 当前本机状态

- GitHub CLI 已安装，版本检查命令为 `gh --version`。
- 当前 PowerShell 会话中如果直接输入 `gh` 找不到命令，可以使用完整路径：

```powershell
& "D:\Code\GitHub CLI\gh.exe" --version
```

- 已确认账号 `YyzhXie` 登录成功，并具备 `repo` 和 `workflow` 权限。
- 当前仓库权限为 `ADMIN`，可以创建提交并推送到远端。

## 登录后的检查流程

每次让 Codex 或自己上传前，建议先做这些非破坏性检查：

```powershell
& "D:\Code\GitHub CLI\gh.exe" auth status
git status -sb
git remote -v
git fetch origin
```

如果需要确认能否推送但不想真的修改远端，可以执行：

```powershell
git push --dry-run origin HEAD
```

## 更新当前仓库

适用于已经在本地仓库中完成修改的情况：

```powershell
git status -sb
git diff --stat
git add <需要提交的文件>
git commit -m "Update documentation and comments"
git push origin master
```

如果整棵工作区都是本次任务范围内的修改，也可以用：

```powershell
git add -A
```

但在混合工作区中应优先显式添加文件，避免把无关改动提交进去。

## 新建仓库并上传

如果以后要从一个新项目文件夹创建 GitHub 仓库，可以使用：

```powershell
git init
git add -A
git commit -m "Initial project"
& "D:\Code\GitHub CLI\gh.exe" repo create <owner>/<repo-name> --public --source . --remote origin --push
```

若希望创建私有仓库，把 `--public` 改为 `--private`。

## 推荐提交习惯

- 先运行能跑的测试，再提交。
- README、接线说明、演示清单和代码同步更新，避免“代码能跑但讲不清”。
- 每个提交只做一类事情，例如“集成 Keil 工程”或“更新文档与注释”。
- 推送前用 `git log -1 --oneline` 记下版本号，便于答辩或回滚。
