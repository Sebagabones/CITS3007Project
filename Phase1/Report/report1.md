Group Name: TOCTOU

NOTE:
- Ensure that the name and student number of all group members are included in the
submission.

- Ensure that the group name and number are included in the submission.

## 1. Team Communication & Responsibilities

### 1.1 Team Communication
The team had our first meeting on Wednesday (the 09/04/2025) at 12:00 (funnily enough, we all happened to be available at 12-2pm on a Wednesday, sorry Arran), where we decided to continue to meet at the same time each week, in person.

If deemed necessary as the project progresses, we agreed to meet more often, but a minimum of once a week seemed like a suitable choice for maintaining the delicate balance between ease of tracking progress without impeding the Computer Science students very (very) busy social lives.


We quickly realised that our form of communication for outside of our meetings required decent support across different Operating Systems, while maintaining usability (ruling out IRC), and so our group settled on Discord as a suitable option.


### 1.2 Team Responsibilities
A main focus of the first meeting was working out the different skillsets of each group member, to allow for a more efficient allocation of tasks.

We soon noted that the group had a varying level of technical skill, which helped with working out who worked on which parts of the second phase of the project.

That said, everyone will, of course, be expected to write code, and so a preliminary division of labour was allocated as so:

- **Peter & Henry**: *Admin and operations access*
- **Seb**: *Session management*
- **Alec**: *Role-based access control*
- **Scotty**: *Player authentication*

*(Note that this is only the allocation of the coding related tasks, other responsibilities are discussed later)*


The division of coding responsibilities was mostly decided as a group, with a preference given to which area everyone wanted to work on.

This division is subject to change, once the group have more information as to the requirements of the project, however it provides a nice starting point for our group to have an idea of what each person is working on.


#### 1.3 Other Responsibilities
Of course, a group project wouldn't be complete without the addition of other requirements and responsibilities, and so some other tasks are listed below:

- **Peter**: Developing testing for the system, which will involve working closely with Alec and Scotty.
- **Henry**: Assisting with documentation, and ensuring tasks are carried out (and lending a hand to any tasks if needed).
- **Seb**: Management, coordinating team efforts, setting up version control system/linting, and documentation.
- **Alec**: Taking minutes at each meeting, threat modelling
- **Scotty**: Design, working on how secure coding principles can be used in the project


There are undoubtedly areas that we have missed in the above list that we will realise once we start phase 2 of the project, and so a critical element to our teams success is being able to adapt to new (or previously unrealised) requirements.


### 1.4 Progress Tracking and Accountability
Arguably the biggest part to a successful group project is effective progress tracking, and consistent accountability, and so we knew to focus on this at the first meeting.

We decided a solution to this was by having a group member who would be responsible for ensuring tasks aren't falling behind, and if they are, alerting the team, and being able to jump on said tasks (by having a reduced amount of "set" tasks) to assist with the tasks being completed on time.

A shared Google Docs spreadsheet was also created, with the purpose to help Henry, and the group as a whole, keep track of the progress of each task.


Finally, meeting in person once a week was deemed the best way to keep accountability, as our group felt that a regular face-to-face meeting would help to remind us about who we would be letting down (as well as making it easier for Henry to reach out to any members who might need assistance with finishing tasks).

Of course, our group also understands that life can get very hectic, and the main request we had for eachother is that if we start falling behind, to let the other group members know immediately, so that we can adjust tasks and priorities before it gets out of hand.


## 2. Version Control Strategy
### 2.1 Remote Source Code Location

Our group decided that GitHub would be a suitable main online VCS provider to host our repository, mostly due to members familiarity with it (and BitBucket's dark-mode *still* being under beta testing).

GitHub also provides GitHub Actions, which one of our group members has experience with, which will be discussed later on in this report in [Part 3.2](#3.2-additional-tooling).

We considered mirroring our repository to one of our members home-server, which has a simple Git based VCS server, however we decided against this, as we feel confident that if GitHub goes down, we will have much bigger problems than this project.

However, if GitHub *does* go down, but the project is still our biggest concern, we will still have local copies on each of our devices, so providing that every group member does not suffer the loss of their devices, *and* GitHub also goes down *all at the same time*, we will then migrate the repository to the aforementioned group members Git server (in which case, we will be using [Soft Serve](https://charm.sh/blog/self-hosted-soft-serve/) as our main VCS server)

Finally, we decided to go with Git as the underlying VCS, even though there are (realistically) much better options, (see [Jujutsu](https://github.com/jj-vcs/jj?tab=readme-ov-file), or [Pijul](https://nest.pijul.com/pijul/pijul), along with the classics such as Fossil, Darcs, or Mercurial), due to Git's extreme popularity (and 4/5 group members unwillingness to not have to learn how to use a new SCM system while also working on the project), we decided to just use Git.

### 2.2 Merging Strategies

Each group member's GitHub account was added to the repository, and at the first meeting the version control fanatic discussed what their planned approach was, requesting feedback.

The overall system will be to use separate branches for each large addition to the project, however each group member is also allowed to have a *up to date* branch specifically for the addition of smaller changes, e.g. fixing typos in documentation etcera. Each branch, when ready, will then have a pull request created, to merge it into the main/dev branch (or any other branch being worked on by a separate member).


To mitigate against merge conflicts, a few strategies were implemented. Regular communication (and a clear layout of responsibilities as listed [above](#1.2-team-responsibilities)) helped to ensure there were no major overlapping code changes. If this was unavoidable (for example, in the case of Peter and Henry), regular merging will be implemented.

For features/branches that are worked on by a single person, regularly merging main into the branch is planned.

If merge conflicts *do* occur (this more of a case of when not if we are being honest), the team will work together to resolve them, seeking assistance from the person(s) most familiar with the conflicting code if possible.


In the absolute worse case, we plan to implement the xkcd/1597 [approach](https://xkcd.com/1597/), however, we are all collectively hoping this will not be necessary.

### 2.3 Branching Systems
Since the group's main SCM enjoyer usually uses [Sapling](https://sapling-scm.com/), they did not have really have an opinion on branching strategies, so our group kept it simple, by having a main branch (creatively named "main"), and a "dev" branch, which will be used for testing.

### 2.4 VCS Policies
The VCS policies our group decided on were not overly complex, however they were intentionally quite restrictive.

At least one review (and approval) is required before the branch can be merged, the branch being merged into the main and/or dev branch(es) *must* be up to date with the destination branch, and the GitHub Actions discussed [below](#3.2-additional-tooling) must also pass before the branch is able to be merged.


Commit messages should be kept to the recommended limit (of 72 characters), with a clear descriptions to be placed in the commit description.

However, this will not be enforced, and group members are encouraged to prioritise clarity over brevity.


## 3. Development Tools

### 3.1 Editors

There will be two main editors in use for this project, Visual Studio Code (VS Code), and Emacs.

The majority of the group plan to use VS Code for this project, as it is what they are used to and are comfortable using, and it has a wide plugin ecosystem.

However, one group member uses Emacs, mostly because they are used to it, and enjoys the extensibility of it.

The group discussed this whether using different editors would cause any issues, however the Emacs user was confident that they would be able to replicate any necessary features in Emacs, and if not will (begrudgingly) switch over to using VS Code for the remainder of the project.

### 3.2 Additional Tooling
This will be split up into 4 main sections - formatting, static analysis/linting, dynamic analysis, and CI/CD implementing both of these.

*Note: The group was absent from the lecture covering Fuzzing (due to attending the first meeting for this project), and so implementing fuzzing has not been discusseed at a group meeting yet, this is planned to be addressed and implemented at the second meeting, which is the day that this report is due, and so the usage of fuzzing is not covered in this report, however it will almost definitely be used in the testing of this project*


#### Formatting
Consistent formatting is important to ensure a codebase remains readable, and so [Uncrustify](https://github.com/uncrustify/uncrustify/) was setup for formatting our all of code. Somewhat controversially, the style used was derived from an Allman/BSD Style.

An argument for as to why this was chosen could be made that due to the brackets being on a separate line to the control statement that commenting out a block of code/control statement while refactoring is less likely to cause errors via missing or dangling braces.

However, this is somewhat mitigated by the modern editors (and Emacs) being used by the group, so a more likely reason for as to why this style was used is that the group member who setup the CI/CD for the project prefers Allman style indentation, and  no-one in the group felt strongly enough about that member's questionable stylistic choices to change it.


Plugins for Uncrustify exist for both VS Code and Emacs, however in case these were not used, more methods for ensuring formatting are explained below

#### Static Analysis/Linting
While Linters and Static Analysis are different tools, they have some cross over, with some tools being marked as both. Hence, both will be discussed here.

Our strategy of choosing tooling was very much based around "how many tools can we use", and so we used as many as possible (while ensuring the group understood how to use them effectively).

##### Linters:
- clang-tidy
  - Using compiler flags such as `gcc -Wall` provide linting (and are still a crucial part of writing secure code), clang-tidy was chosen to be used due to its deeper (and more modern) analysis.
  - Clang-tidy's ability to analyse for undefined behaviour provides much more in depth warnings of unsafe code, increasing the safety of our code.
  - It also does a nice job of fixing errors where it can, which saved a considerable amount of time.
  - The flags used by our group were `--fix-errors -warnings-as-errors=*`
- cpplint
  - While this might seem like an odd choice when using Uncrustify (which did involve adding a considerable number of flags to cpplint to prevent conflicts between Uncrustify and it), cpplint also enforces function size limits, code aware indentation, which can prevent hidden logic bugs.
  - It also flags style violations, unlike Uncrustify, which our team found useful.
  - There were a lot of flags used here to prevent conflicting with Uncrustify, however the main focus was on the `+runtime` and `+build` filters, which targetted things that Uncrustify doesn't

##### Static Analysis Tools:
- cppcheck
  - This was chosen as it was a lot easier for our group to setup and use effectively than something like Splint, while still providing an effective anaylisis to warn of common issues that aren't always picked up by compilers.
  - The flag used was `--check-level=exhaustive`, however this was mostly for pre-commit setup (more on that later).
- flawfinder
  - We found flawfinder very helpful for explaining the context of any issues, which helped us quickly deem if something was a necessary problem.
  - It provided a very useful first line of checking where problems may be (such as using unsafe functions), before using static heavier tools for deeper auditing.

Finally, as it is worth mentioning here, before going into more detail of the process later, we had a vast number of compiler flags and warnings designed to help us be warned of any issues we may face before running linters and static anaylisis tools.

#### Dynamic Analysis
We implemented a few different tools for this, however our main approach was to use Sanitizers and Valgrind.

##### Sanitizers
We plan to use the three main Google Sanitizers (Address, Memory, Thread), as in our environment we can control the compilation flags. These will provide us with v

That said, we
