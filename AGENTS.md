# Enhance Vocabulary Agent Instructions

## Mandatory startup procedure

Before inspecting, planning, reviewing, or changing this project:

1. Read this entire `AGENTS.md`.
2. Read the entire root `README.md`.
3. Check the current Git status.
4. Treat existing working-tree changes as user-owned.
5. Confirm the approved scope before modifying anything.

Do not begin implementation until these steps are complete.

## Primary rule

NO UNAPPROVED CHANGES ARE ALLOWED.

`AGENTS.md` is the authoritative working agreement. If instructions conflict, stop and ask the user.
An agent must change only the files and behavior explicitly approved by the user.

- Inspect and understand relevant code before proposing changes.
- Confirm the intended scope before editing.
- If the scope is ambiguous, stop and ask for clarification.
- Preserve the existing architecture, module boundaries, naming conventions, file locations, and project structure.
- Do not perform nearby cleanup, refactoring, renaming, reformatting, dependency changes, or structural improvements without separate approval.
- Never overwrite or discard pre-existing user changes.
- Do not stage, commit, push, move, rename, or delete files unless explicitly requested.
- Report exactly which files were changed and how the result was verified.

Approval for one change does not imply approval for related changes.

## Repository scope

The repository root is the directory containing:

- `EnhanceVocabulary.uproject`
- `Source/`
- `Content/`
- `Config/`

Primary project areas:

- `Source/` - C++ modules and Android integration
- `Content/` - Unreal assets, Blueprints, widgets, maps, and materials
- `Config/` - Unreal project configuration
- `Build/` - project build resources
- `README.md` - architecture and developer workflow

Generated or machine-local directories:

- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`

Do not edit or treat generated-directory contents as project source unless the user explicitly approves a targeted operation there.

## Required workflow

1. Check Git status and inspect the relevant files.
2. Identify existing conventions and dependencies.
3. Confirm that the proposed change is inside the approved scope.
4. Make the smallest change that satisfies the request.
5. Avoid unrelated formatting and cleanup.
6. Verify only to the extent authorized by the user.
7. Report changed files, verification performed, and any limitations.

Documentation-only approval does not authorize:

- C++ changes
- Blueprint or asset changes
- configuration changes
- module dependency changes
- builds or packaging
- Jira updates
- Git staging, commits, or pushes

## Module ownership

Follow the existing ownership model:

- `EnhanceVocabulary` - application composition and top-level Unreal application classes
- `EnhanceVocabularyCore` - shared domain types, interfaces, and general utilities
- `EnhanceVocabularyUI` - UMG widget behavior and presentation code
- `EnhanceVocabularyWeb` - HTTP providers, connectivity, requests, and response parsing
- `EnhanceVocabularyStorage` - SQLite persistence and import/export behavior
- `EnhanceVocabularyDevice` - platform-specific and Android integration

Put new code in the narrowest appropriate module.

Do not move existing code between modules or alter module dependencies without explicit approval.

## Application responsibility and communication contract

Before inspecting, planning, or changing application composition, UI ownership, widget communication, Player Controller, Game Instance, or module boundaries, read the entire `Docs/Architecture/Application_Responsibility_and_Communication_Model.md`.

The approved communication chain is:

```text
Child <-> Feature <-> Root <-> Player Controller <-> Game Instance <-> capability implementation
```

- Communicate only between immediate neighboring layers through stable, role-based contracts.
- Do not introduce skipped-layer access, concrete descendant control, sibling control, or a global event bus.
- Name contracts after intent, capability, or state rather than a concrete widget, control, layout, or provider.
- Optional Features, Children, and capability implementations must be removable without breaking unrelated behavior.
- Keep shared contracts implementation-neutral and in the narrowest appropriate module.

The architecture document defines the target model. It does not authorize broad migration or incidental refactoring. Any exception or adoption work requires explicitly approved files and behavior.

## Mandatory UI binding and behavior-preservation rules

These rules are mandatory for every UI change, migration, refactor, and architectural adoption task:

- Never rename a widget bound through `BindWidget` or `BindWidgetOptional`. Preserve both the C++ property name and the corresponding Widget Blueprint widget name, including spelling and casing.
- Do not change the existing logic, meaning, observable behavior, payload, sequencing, or side effects of any function, event, delegate, or interface unless it contradicts the approved application responsibility and communication contract.
- Replacing a prohibited communication path must preserve the behavior of the existing function, event, delegate, or interface. Prefer adapters and forwarding boundaries over redesigning existing behavior.
- Do not rename, consolidate, remove, repurpose, or otherwise clean up existing functions, events, delegates, or interfaces as part of a migration.
- If existing logic contradicts `Docs/Architecture/Application_Responsibility_and_Communication_Model.md`, stop before changing it. Report the exact file, symbol, current behavior, contradiction, proposed change, and expected impact, then wait for explicit user approval.
- A contradiction with the architecture document establishes a need for discussion; it does not authorize a change by itself.

Approval to migrate a communication path does not imply approval to alter the behavior carried by that path.

## UI and reusable types

- Keep widget-specific behavior and presentation types in `EnhanceVocabularyUI`.
- Keep visual layout, styling, animation, and editor bindings in Blueprint/UMG assets.
- Put a type in `EnhanceVocabularyCore` only when it is genuinely shared across modules.
- Do not move UI-only types into Core merely to make them globally accessible.
- Expose only the Blueprint-facing API required by actual Blueprint consumers.

Moving behavior between C++ and Blueprint is an architectural change and requires approval.

## Verification

Use the smallest verification appropriate to the approved work.

- Documentation: review the diff and validate referenced names and paths.
- C++: compile the relevant Unreal target when authorized.
- UI/runtime behavior: perform a targeted Play In Editor check when authorized.
- Android behavior: validate on an Android device when authorized.

Do not claim Android-specific behavior was verified through a desktop or editor-only test.

If verification requires an unapproved action, stop and request permission.