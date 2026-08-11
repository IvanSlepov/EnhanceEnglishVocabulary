# Enhance Vocabulary

Enhance Vocabulary is a custom Unreal Engine 5.7 Android application for storing English vocabulary and displaying words on the phone at a user-defined frequency.

## Requirements

- Unreal Engine 5.7
- A C++ toolchain supported by the installed Unreal Engine version
- Android SDK/NDK tooling compatible with the project settings when building for Android

Enabled project plugins include:

- `SQLiteSupport`
- `SQLiteCore`
- `JsonBlueprintUtilities`
- `ModelingToolsEditorMode` for editor targets

## Repository structure

- `Source/` - C++ modules, Unreal targets, and Android integration
- `Content/` - maps, Blueprints, UMG widgets, materials, images, and data assets
- `Config/` - Unreal Engine, game, editor, input, and platform settings
- `Build/` - project build resources
- `EnhanceVocabulary.uproject` - project, runtime-module, and plugin declarations

The following are generated or machine-local directories rather than architectural source directories:

- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`

## C++ modules

The project contains six runtime modules.

### `EnhanceVocabulary`

The application and composition module. It owns top-level Unreal application classes such as the game instance, game mode, player controller, and primary application module. It composes services provided by Core, Web, Storage, and Device.

### `EnhanceVocabularyCore`

The shared foundation module. It contains reusable vocabulary and language types, action and status types, provider interfaces, validation helpers, JSON helpers, and general cross-module utilities.

Core should remain independent of feature-specific implementations in UI, Web, Storage, Device, and the application module.

### `EnhanceVocabularyUI`

The C++ UI module. It owns UMG widget base classes, widget behavior, display formatting, filtering and translation controls, and C++ integration with assets under `Content/Widgets`.

### `EnhanceVocabularyWeb`

The web-services module. It owns connectivity checks, HTTP requests, provider registration, provider URL construction, response parsing, and word-search orchestration.

### `EnhanceVocabularyStorage`

The persistence module. It owns SQLite schema and query code, vocabulary storage, import/export processing, import validation, and validation-report formatting.

### `EnhanceVocabularyDevice`

The platform-integration module. It owns platform file exchange, Android JNI integration, Unreal Plugin Language integration, notification and alarm behavior, and editor or unsupported-platform implementations.

## Current module dependencies

The `.Build.cs` files are the source of truth for compile-time dependencies.

The current project-level feature dependency graph is:

```text
EnhanceVocabularyCore
    ^
    |-- EnhanceVocabularyWeb
    |-- EnhanceVocabularyStorage
    |-- EnhanceVocabularyDevice

EnhanceVocabulary
    |-- EnhanceVocabularyCore
    |-- EnhanceVocabularyWeb
    |-- EnhanceVocabularyStorage
    `-- EnhanceVocabularyDevice

EnhanceVocabularyUI
    |-- EnhanceVocabulary
    |-- EnhanceVocabularyCore
    |-- EnhanceVocabularyWeb
    `-- EnhanceVocabularyStorage
```

This documents the existing graph. It is not authorization to reorganize it.

When adding code:

- Prefer the narrowest owning module.
- Do not introduce a reverse dependency into Core.
- Do not add module dependencies merely for convenience.
- Review whether a dependency belongs in the public or private dependency list.
- Treat dependency corrections and module moves as separately approved architectural work.

## C++ and UI asset separation

C++ classes in `EnhanceVocabularyUI` own reusable widget behavior, state coordination, validation, formatting, and service interaction.

Assets under `Content/Widgets` own Blueprint composition, visual layout, styling, animation, and editor-configured bindings.

General rules:

- Shared vocabulary and domain types may belong in Core when they have multiple module consumers.
- UI-only data and styling types should remain in UI.
- UI-only types should not be moved into Core solely to make them globally accessible.
- Blueprint-visible types should expose only the API needed by their Blueprint consumers.
- Moving existing behavior between C++ and Blueprint requires architectural review and explicit approval.

## Naming conventions

Follow the conventions already present in the project:

- `EV` - Enhance Vocabulary project identifier
- `UEV...` - Unreal `UObject`-derived classes
- `AEV...` - Unreal actor-derived classes, including controllers and game modes
- `FEV...` - structs and non-`UObject` value types
- `EEV...` - enumerations
- `WBP_...` - Widget Blueprint assets
- `BP_...` - Blueprint assets
- `DA_...` - Data Asset instances
- `M_...` - materials
- `MI_...` - material instances
- `MF_...` - material functions
- `ENHANCEVOCABULARY..._API` - module export macros

Preserve existing filenames, symbol names, directory locations, and casing.

Do not rename existing code or assets as incidental cleanup.

## Build procedure

Open `EnhanceVocabulary.uproject` using Unreal Engine 5.7 and allow Unreal to compile the project modules when prompted.

The project includes these targets:

- `EnhanceVocabulary` - game target
- `EnhanceVocabularyEditor` - editor target

For command-line compilation on Windows, use the build script belonging to the local Unreal Engine installation:

```powershell
<UE_ROOT>\Engine\Build\BatchFiles\Build.bat EnhanceVocabularyEditor Win64 Development "<REPOSITORY_ROOT>\EnhanceVocabulary.uproject" -WaitMutex
```

Replace:

- `<UE_ROOT>` with the local Unreal Engine 5.7 installation directory
- `<REPOSITORY_ROOT>` with the absolute repository path

Do not assume that every developer uses the same Unreal Engine installation directory.

## Verification procedure

After a successful editor build:

1. Open the project in Unreal Editor.
2. Confirm that it loads without new module or Blueprint compilation errors.
3. Run the relevant workflow in Play In Editor when runtime behavior was changed.
4. Verify only the functionality affected by the approved change.

For Android or platform-specific changes:

1. Build the Android target.
2. Install it on an appropriate Android device.
3. Test the affected platform workflow.
4. Inspect relevant Unreal and Android logs.

An editor-only check does not verify:

- JNI integration
- UPL changes
- Android permissions
- alarms
- notifications
- Android file exchange

## Automated tests

No Unreal Automation Test suite is currently present under `Source`.

Until an automated test suite is introduced through an approved task, verification consists of:

- compilation
- targeted Unreal Editor smoke testing
- targeted Android device testing when platform behavior is affected

Do not claim that device-specific behavior was verified when only a desktop build was performed.

## Change-scope rule

NO UNAPPROVED CHANGES ARE ALLOWED.

Every change must stay within the explicitly agreed files and behavior.

Preserve the existing structure and request clarification before performing:

- adjacent cleanup
- refactoring
- renaming
- reformatting
- dependency changes
- module moves
- Blueprint or asset reorganization
- unrelated fixes

See `AGENTS.md` for the complete agent working agreement.