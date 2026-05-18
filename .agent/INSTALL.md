# Install Agent Profile for SsalMuk

This project includes an agent profile under `.agent` that describes agent execution rules and available tools.

## Usage

- Keep `.agent/AGENTS.md` updated with the project-specific agent workflow.
- Use `docs/plans/task.md` for live task tracking.

## Expected tools

- `read_file`
- `list_dir`
- `grep_search`
- `create_file`
- `create_directory`
- `replace_string_in_file`
- `multi_replace_string_in_file`
- `run_in_terminal`
- `browser_subagent` (browser automation only)

## Verification

No special runtime installation is required for the documentation files themselves.

If you want to validate that the profile is present, confirm `.agent/AGENTS.md` exists and `docs/plans/task.md` is available for task tracking.
