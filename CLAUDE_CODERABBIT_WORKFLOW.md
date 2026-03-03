# Claude + CodeRabbit Workflow

## How Claude and CodeRabbit Work Together

This document explains how to use Claude Code with CodeRabbit for AI-assisted development and review.

## Workflow Overview

```
You (Human)
    ↓
Claude Code (Implementation)
    ↓
GitHub PR
    ↓
CodeRabbit (Review)
    ↓
Claude Code (Address feedback)
    ↓
Merge!
```

## Step-by-Step Process

### 1. Request a Feature from Claude

**You say:**
> "Implement user onboarding with TDEE calculation"

### 2. Claude Creates Implementation

Claude will:
- Create feature branch
- Write the code
- Add tests
- Commit changes
- Push to GitHub
- Create PR

### 3. CodeRabbit Reviews Automatically

CodeRabbit will:
- Analyze the code
- Check for bugs, security issues, best practices
- Post review comments on the PR
- Suggest improvements

### 4. Claude Addresses Feedback

**You say:**
> "Address CodeRabbit's feedback on PR #2"

Claude will:
- Read CodeRabbit's comments
- Fix the issues
- Push updates
- CodeRabbit re-reviews automatically

### 5. Merge When Approved

When CodeRabbit approves and CI passes:
- Claude (or you) merges the PR
- Branch auto-deletes
- Feature is in develop!

## Commands You Can Use

### Trigger CodeRabbit Review
```
@coderabbitai review
```
Use this in PR comments to manually trigger a review.

### Ask CodeRabbit Questions
```
@coderabbitai Can you explain this function?
@coderabbitai Is there a security issue here?
@coderabbitai Suggest improvements for performance
```

### Configure CodeRabbit
```
@coderabbitai configure
```
Opens configuration options.

## Example Conversation

**You:**
> "Implement the workout tracking service with PR detection"

**Claude:**
> "I'll implement that. Creating feature branch and building the service..."
>
> *[Claude writes code, creates PR]*
>
> "PR #2 created: https://github.com/.../pull/2"
> "CodeRabbit will review automatically."

**[CodeRabbit reviews and comments]**

**You:**
> "Fix the issues CodeRabbit found"

**Claude:**
> "I'll address CodeRabbit's feedback..."
>
> *[Claude reads CodeRabbit's comments, fixes issues, pushes]*
>
> "Updates pushed. CodeRabbit is re-reviewing now."

**[CodeRabbit approves]**

**You:**
> "Merge it"

**Claude:**
> "PR #2 merged! Workout tracking is now in develop."

## Benefits of This Workflow

### 🤖 Two AI Layers
- **Claude**: Implements features quickly
- **CodeRabbit**: Reviews for quality, security, bugs

### 🚀 Fast Iteration
- Claude can address feedback instantly
- No waiting for human reviewers
- Learn best practices from CodeRabbit's suggestions

### 📚 Learning Tool
- See CodeRabbit explain WHY something is wrong
- Claude incorporates feedback for future code
- Improve your codebase quality over time

### ✅ Quality Assurance
- Catch bugs before they reach production
- Security vulnerabilities detected early
- Consistent code style enforced

## Tips for Best Results

### Be Specific with Claude
❌ "Add a feature"
✅ "Add user authentication with JWT tokens and bcrypt password hashing"

### Let CodeRabbit Teach
- Read CodeRabbit's explanations
- Ask follow-up questions
- Apply learnings to future PRs

### Use Both AIs
- Claude for rapid implementation
- CodeRabbit for quality review
- You for final decision-making

## Current Configuration

CodeRabbit is configured to:
- ✅ Auto-review all PRs
- ✅ Work on all branches (main, develop, feature branches)
- ✅ Focus on security, performance, best practices
- ✅ Explain issues clearly
- ✅ Suggest specific improvements

## Common Scenarios

### Scenario 1: New Feature
```
You: "Implement nutrition tracking with USDA API"
Claude: [Creates PR]
CodeRabbit: [Reviews, finds API key exposure risk]
You: "Fix the security issue"
Claude: [Fixes, pushes]
CodeRabbit: [Approves]
You: "Merge"
```

### Scenario 2: Bug Fix
```
You: "Fix the authentication bug in login.tsx"
Claude: [Creates PR with fix]
CodeRabbit: [Reviews, suggests edge case handling]
Claude: [Adds edge case handling]
CodeRabbit: [Approves]
Merge!
```

### Scenario 3: Refactoring
```
You: "Refactor the profile service for better performance"
Claude: [Creates PR with optimizations]
CodeRabbit: [Reviews, suggests caching strategy]
You: "Implement the caching suggestion"
Claude: [Adds caching]
CodeRabbit: [Approves]
Merge!
```

## Getting Help

**From Claude:**
- Ask Claude to implement features
- Ask Claude to fix CodeRabbit's feedback
- Ask Claude to explain CodeRabbit's suggestions

**From CodeRabbit:**
- Use `@coderabbitai explain` for clarification
- Use `@coderabbitai suggest` for alternatives

**From Humans:**
- Review PRs before merging (optional but recommended)
- Make final decisions on architectural choices
- Approve/reject based on business needs

## Remember

- ✅ Always create PRs (never push to main)
- ✅ Let CodeRabbit review before merging
- ✅ Address all CodeRabbit comments (or explain why not)
- ✅ Keep Claude in the loop (ask it to fix issues)

---

**This workflow makes you a 10x developer!** 🚀

Claude implements fast → CodeRabbit reviews thoroughly → You ship quality code.
