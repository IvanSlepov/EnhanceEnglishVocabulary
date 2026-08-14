# Enhance Vocabulary Application Responsibility and Communication Model

## Status and authority

This document defines the approved target architecture for application composition, UI ownership, and communication between application layers. It is a permanent project architecture contract, not a point-in-time audit.

The central rule is:

```text
Child <-> Feature <-> Root <-> Player Controller <-> Game Instance <-> capability implementation
```

Each layer communicates only with its immediate neighbor. Communication in either direction uses stable, role-based contracts rather than knowledge of a concrete widget, layout, provider, or service implementation.

This document does **not** authorize a repository-wide migration. Existing code that does not yet follow this model must be changed only through separately approved Jira work and explicitly agreed files. Do not perform adjacent refactoring, renaming, dependency changes, module moves, or Blueprint reorganization while adopting this model.

## Architectural objective

Enhance Vocabulary should behave like an attachment-oriented system. Features and capability implementations should be attachable, replaceable, and removable without forcing the surrounding application to be redesigned.

Removing an optional end component should make only its capability unavailable. It must not break application startup, unrelated features, or the communication chain. Replacing a component should require a new implementation of the same stable contract, not caller renaming or broad routing changes.

The architecture separates:

- **ownership** - which layer owns lifetime, state, and composition;
- **communication** - which neighboring layer may exchange an intent, command, query, event, or outcome;
- **implementation** - which concrete widget or module performs the work.

## Operating principles

1. Communicate only with the immediate neighboring layer.
2. Depend on stable roles and capabilities, not concrete widgets or services.
3. Each object executes its own internal methods in response to typed communication.
4. Parents coordinate immediate children but do not reach through them to descendants.
5. Siblings do not control one another directly.
6. Optional attachments may be absent without breaking unrelated behavior.
7. Do not use a global event bus to bypass visible ownership.
8. Keep application and UI composition extendable without adding speculative modules now.

## Layer responsibilities

### Child

A Child is an immediate visual or interaction component inside a Feature.

A Child:

- owns local visual behavior and interaction state;
- emits semantic events describing user intent or completed local interaction;
- consumes state or commands through a stable role-based contract;
- does not know Root, Player Controller, Game Instance, or a capability implementation;
- does not communicate directly with siblings;
- does not require its parent API to contain its concrete class or visual name.

A Feature may coordinate its immediate Children. It must not reach through a Child to manipulate grandchildren.

### Feature

A Feature represents an application-facing UI capability or screen, such as Add Word, Review Words, Entry Details, Settings, Import/Export, or Notifications.

A Feature:

- owns its local workflow and presentation state;
- owns and initializes its immediate Child hierarchy;
- translates Child events into feature-level semantic intents;
- applies feature-level outcomes received from Root;
- exposes a stable contract independent of its internal widget tree;
- remains replaceable without requiring Root to learn its internal controls;
- handles missing optional Children safely.

A Feature does not call Player Controller, Game Instance, or a concrete application service.

### Root

Root is the commanding and composition point for application feature UI.

Root:

- owns the main feature switcher and feature navigation;
- registers available Features and tracks their availability;
- initializes and coordinates Features without knowing their internal Children;
- routes semantic Feature intents toward Player Controller;
- routes application outcomes to the responsible Feature;
- derives navigation availability from registered capabilities;
- owns feature-level views, including Entry Details, rather than delegating them to Player Controller;
- does not access Game Instance or a capability implementation directly.

Root may delegate registration, navigation, and routing mechanics to focused helpers within `EnhanceVocabularyUI`, while remaining the UI composition authority.

### Player Controller

Player Controller is the single UI-to-application gateway.

Player Controller owns:

- creation and retention of Root;
- binding the Root/Application communication boundary;
- global dialogs and confirmations;
- global errors;
- global status and loading presentation;
- routing UI intents to Game Instance;
- routing application outcomes back to Root.

Player Controller does not own:

- the Entry Details Feature;
- feature navigation or feature-local state;
- concrete Child widgets;
- vocabulary editing or filter state;
- import/export workflow state;
- notification workflow state;
- relation or translation workflow state;
- direct references used to manipulate Feature internals.

Focused helpers in `EnhanceVocabulary` should hold cohesive gateway, dialog, overlay, or workflow responsibilities when introduced through approved work. Helpers unload the commanding class; they do not create a second communication path around it.

### Game Instance

Game Instance is the application composition root and commanding point for long-lived application capabilities.

Game Instance:

- owns or composes application services and workflow coordinators;
- translates Player Controller requests into capability operations;
- returns capability outcomes through Player Controller;
- owns application-lifetime state that is not presentation state;
- does not know concrete widgets or manipulate UI state;
- delegates cohesive workflows to focused helpers rather than growing into a second God class.

### Capability implementation

Capability implementations include the existing Web, Storage, and Device modules and may later include independently owned AI, Sync, or other modules.

A capability implementation:

- implements a stable contract shared through the appropriate boundary;
- owns its internal state and implementation details;
- has no knowledge of UI composition;
- can be substituted without changing UI contracts;
- reports unavailable or unsupported behavior through a neutral outcome.

## Module roles

The communication model does not replace the established module ownership model:

- `EnhanceVocabularyCore` owns genuinely shared, implementation-neutral contracts, identifiers, payloads, and domain types.
- `EnhanceVocabularyUI` owns Root, Features, Children, UI routing helpers, and presentation behavior.
- `EnhanceVocabulary` owns Player Controller, Game Instance, application composition, and their focused helpers.
- `EnhanceVocabularyWeb`, `EnhanceVocabularyStorage`, and `EnhanceVocabularyDevice` own their current capability implementations.
- Future AI, Sync, or other capability modules should depend on the narrow shared contracts they implement and attach through application composition.

Core is the contract floor, not a universal dumping ground. UI-only types remain in UI. Implementation-specific service types remain in their owning module. A type belongs in Core only when multiple modules genuinely require a neutral shared definition.

The communication chain describes runtime responsibility. It must not be implemented by introducing circular compile-time module dependencies. Dependency changes require separate architectural approval and build verification.

## Communication semantics

Use typed communication with explicit payloads:

- **intent** - a request originating from user interaction;
- **command** - an explicit instruction to a neighboring role;
- **query** - a request for current information with a defined owner;
- **event** - a fact that has occurred;
- **outcome** - a result, failure, cancellation, unavailable state, or asynchronous completion.

Events are preferred for interaction, state changes, and asynchronous outcomes because the sender does not need to control the receiver's internal method sequence. A narrow interface call is acceptable for explicit lifecycle operations, commands, or synchronous queries between immediate neighbors. Neither form permits a layer to call a non-neighbor or depend on a concrete descendant.

Do not introduce a global event bus. Communication remains typed, bounded, and hierarchical so ownership and event lifetime stay visible.

Asynchronous operations should carry enough request identity and context to prevent unrelated in-flight workflows from overwriting or consuming one another's state.

## Naming rules

Contract names describe intent, capability, or state. They do not describe a concrete widget, button, visual hierarchy, or current destination implementation.

Use these conventions:

- intents use `Requested`, for example `OnEntryDetailsRequested`;
- events and outcomes describe facts, for example `OnVocabularyChanged` or `OnVocabularySearchCompleted`;
- commands use an imperative capability name, for example `PresentEntryDetails`;
- queries use a neutral information name;
- presentation inputs describe the desired state, not how a specific control achieves it.

Avoid names such as:

- `DisableSearchButton`;
- `SetSearchResultsPanel`;
- `HandleReviewWordsRefresh`;
- `HandleOpenReviewWordsForNotification`.

Prefer names such as:

- `OnVocabularySearchRequested`;
- `OnSearchInteractionStateChanged`;
- `OnEntryDetailsRequested`;
- `OnNavigationRequested`;
- `OnConfirmationRequested`;
- `OnConfirmationResolved`;
- `ApplyVocabularyQueryResult`;
- `PresentEntryDetails`.

Replacing or removing a Child must not force callers to rename their APIs. If a name becomes invalid when a particular widget is replaced, the name exposes implementation detail and does not belong in the stable contract.

## Attachment and availability rules

Every attachment point must distinguish required and optional capabilities.

For optional Features, Children, and capability implementations:

- absence is a supported state;
- missing registration does not fail application startup;
- bindings are null-safe and allow zero listeners where appropriate;
- navigation or controls for an unavailable capability are omitted or disabled deliberately;
- requests return a neutral unavailable or unsupported outcome;
- missing capability diagnostics are logged without repeated noise;
- delegates and callbacks are unbound when an attachment is removed or destroyed;
- unrelated Features continue operating;
- replacement requires only the same contract, registration, asset substitution when applicable, and targeted verification.

Required attachments must be identified explicitly. A component must not become implicitly required merely because a concrete pointer is assumed to exist.

## Reference flow

A vocabulary-details workflow follows the chain without skipped layers:

```text
Child interaction
    -> Feature intent
    -> Root routing
    -> Player Controller application request
    -> Game Instance workflow
    -> Storage capability

Storage outcome
    -> Game Instance outcome
    -> Player Controller UI outcome
    -> Root routing
    -> Feature state update
    -> Child presentation update
```

Each layer interprets the message at its own level and invokes its own internal methods. Player Controller may present a global confirmation, error, status, or loading state during the workflow, but it does not manipulate the Entry Details Feature or its Children.

## Extension model

The project has two composition rails:

1. **Application capability rail** - Game Instance composes Web, Storage, Device, and future AI, Sync, or other capability implementations behind stable contracts.
2. **UI feature rail** - Root composes Add Word, Review Words, Entry Details, Settings, Import/Export, Notifications, and future Feature implementations behind stable contracts.

Adding a future capability should localize change to:

- its owning module;
- its neutral contract and payloads where genuinely shared;
- application composition and registration;
- the Feature that chooses to expose it.

It must not require unrelated widgets or modules to depend on the new implementation.

## Implemented V2 baseline

The following structures implement this contract in the current project:

### UI feature rail

- `UEVRootWidget` is the UI composition authority.
- `UEVFeatureRegistry` registers Feature identifiers against optional widgets in `WidgetSwitcher_Main`, activates available Features, and remembers the previous Feature for temporary views.
- The registered Feature set covers Main Menu, Add Word, Review Words, Notification Settings, Import/Export, Application Settings, Entry Details, and Vocabulary Filters.
- `IEV...FeatureRole` contracts carry semantic Feature intents and presentation state between each Feature and Root.
- Entry Details and Vocabulary Filters are Root-owned Features. Player Controller does not create, remove, or manipulate either widget.
- Main Menu controls are enabled according to registered Feature availability.
- Replaceable Root Features and selected immediate Feature children use optional widget binding while preserving their existing bound names and types.

### Root/Application boundary

- Neutral application ports in `EnhanceVocabularyCore` carry search, library, preferences, connectivity, global presentation, Entry Details, vocabulary filters, file exchange, notification settings, vocabulary-value actions, lifecycle, and Feature navigation messages.
- `AEVAppPlayerController` binds those ports and remains the single Root-to-application gateway.
- Legacy Blueprint-facing delegates and functions remain compatibility adapters. A semantic route is preferred when attached; the compatibility route is used only when the semantic contract is unavailable.
- `EnhanceVocabularyUI` has no compile-time dependency on the `EnhanceVocabulary` application module and contains no Game Instance or Player Controller access.

### Player Controller helpers

Player Controller owns and composes focused helpers within `EnhanceVocabulary`:

- `UEVFileExchangeWorkflowCoordinator` owns UI-originated file-operation workflow state and decisions.
- `UEVNotificationWorkflowCoordinator` owns notification-settings transition and permission workflow state.
- `UEVVocabularyInteractionCoordinator` owns Entry Details edit/delete, relation/translation action, and vocabulary-filter workflow state.

These helpers do not own widgets. They request confirmation, loading, and status presentation through controller-owned signals; Player Controller continues to create and own the global overlays and dialogs.

### Game Instance coordinators

Game Instance owns and composes focused application coordinators:

- `UEVFileExchangeApplicationCoordinator` orchestrates Storage and Device capabilities for template download, export, import, validation reports, and completion outcomes.
- `UEVNotificationApplicationCoordinator` orchestrates Storage and Device capabilities for notification payloads, scheduling, permission state, pending words, and device settings.

Game Instance keeps the controller-facing capability API and is the only owner that composes these coordinators with concrete services. Missing coordinators produce neutral unavailable behavior rather than exposing implementation details to Player Controller.

### Required and optional attachments

`WidgetSwitcher_Main` and the Root shell remain required composition controls. Registered application Features are optional attachments. If an optional Feature is absent:

- registration omits it;
- Main Menu disables its entry when that entry exists;
- navigation rejects the unavailable Feature without changing unrelated state;
- Root and the application continue operating;
- no concrete Feature pointer is exposed to Player Controller or Game Instance.

This baseline does not remove the incremental-adoption rule. Existing compatibility APIs may be removed only through separately approved work after their final Blueprint or C++ consumer is proven gone.

## Prohibited coupling

Unless a separately reviewed architecture decision explicitly changes this model, do not introduce:

- Child-to-Root, Child-to-Player Controller, Child-to-Game Instance, or Child-to-service access;
- Feature-to-Player Controller, Feature-to-Game Instance, or Feature-to-service access;
- Root-to-Game Instance or Root-to-service access;
- Player Controller-to-concrete-Feature or Child manipulation;
- Game Instance-to-widget access;
- capability implementation-to-UI access;
- parent-to-grandchild reach-through;
- sibling-to-sibling direct control;
- globally broadcast events used to bypass ownership;
- contract names tied to replaceable visual components;
- Core dependencies on application, UI, or capability implementation modules.

## Adoption rule

This model is the destination for V2 and the extension point for later versions. Adoption is incremental:

1. Identify the existing interaction and its current owner.
2. Agree on the narrow files and behavior to migrate.
3. Introduce or reuse the smallest stable neighboring-layer contract.
4. Migrate only the approved consumer and workflow.
5. Verify the affected compilation and runtime path.
6. Remove obsolete coupling only when its final consumer is gone and removal is explicitly approved.

Do not convert unrelated widgets or services merely because a new contract exists.

## Review checklist

Before approving an application or UI architecture change, confirm:

- Does every interaction stay between immediate neighboring layers?
- Does the owner of the state remain clear?
- Does the contract name describe capability rather than implementation?
- Can an optional attachment be absent without breaking unrelated behavior?
- Can a replacement implement the same contract without caller renaming?
- Are UI-only and implementation-specific types kept out of Core?
- Are asynchronous requests and outcomes correlated safely?
- Are callbacks unbound with the owning lifetime?
- Does the change avoid a global event bus and compile-time dependency cycle?
- Is the migration limited to the explicitly approved files and behavior?

If any answer is no or uncertain, stop and obtain explicit architectural approval before implementation.
