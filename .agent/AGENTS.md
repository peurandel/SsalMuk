# Agent Profile for SsalMuk

This profile adapts agent execution patterns for the SsalMuk workspace with a strict single-flow task model.

## Core Rules

1. Prefer project-local skills in `.agent/skills/<skill-name>/SKILL.md`.
2. Execute one core task at a time using explicit task boundaries.
3. Use `browser_subagent` only for browser automation tasks.
4. Track task progress in `docs/plans/task.md` as a list-only table.
5. Keep changes scoped to the requested task and verify before claiming completion.

## Tool Translation Contract

Use these workspace-appropriate tools when handling agent tasks:

- File read / view -> `read_file`
- Directory listing -> `list_dir`
- Search -> `grep_search`
- Code / file edits -> `replace_string_in_file`, `multi_replace_string_in_file`, `create_file`
- File creation -> `create_file`
- Directory creation -> `create_directory`
- Terminal / shell execution -> `run_in_terminal`
- Browser automation tasks -> `browser_subagent`
- User-facing updates -> `notify_user`

## Skill Loading

- First preference: local skills under `.agent/skills/`.
- Second preference: user/system skills if available.
- If both exist, project-local skills win for this profile.

## Single-Flow Execution Model

- Do not dispatch multiple coding agents in parallel.
- Decompose work into ordered, explicit steps.
- Keep exactly one active task at a time in `docs/plans/task.md`.
- Use browser automation only within a dedicated browser step.

## Verification Discipline

Before reporting completion:

1. Run the relevant verification command(s).
2. Confirm exit status and key output.
3. Update `docs/plans/task.md`.
4. Report evidence, then claim completion.
