# `StructuredDataVersions` Update

## Introduction

The `StructuredDataVersions` ("SDV") class already exists as a [data type in Common](https://github.com/maidsafe/MaidSafe-Common/blob/next/include/maidsafe/common/data_types/structured_data_versions.h).  So far, it is the only mutable data type allowed on the network (Public Key types can be invalidated once stored on the network, but are otherwise considered immutable).  They will be handled by the network in a similar way to `ImmutableData` (i.e. they will be stored as individual files on VersionManagers and the location of the VersionManagers will be held by DataManagers).  This document describes changes required to support an updated design of mutable data handling by the network.



## Motivation

The only use of SDVs at the moment is by [`Directory`](https://github.com/Fraser999/MaidSafe-Drive/blob/next/include/maidsafe/drive/directory.h#L116-L122) in Drive and by [`AccountHandler`](https://github.com/Fraser999/MaidSafe-API/blob/next/include/maidsafe/detail/account_handler.h#L68) in API.  In both cases, the class to be updated is encrypted, stored to the network as an `ImmutableData` chunk, and the associated SDV gets a new version added which contains the name of this immutable chunk.  Essentially, the SDV contains a tree of pointers to the actual data rather than a tree of the data itself.

The design updates to SDV described here will allow this process to be made more efficient.  It will also allow the SDV class to be more widely used (e.g. for safecoins or MPID public data) and will make the SDV much more validatable.



## Overview

The main changes to the SDV class' public API will be:

- the addition of an optional "Owner ID" field (a 64 byte string)
- the addition of a collection of "Other IDs" (each being a 64 byte string)
- the tree of `VersionName`s will be changed to a tree of `Version`s
- there will be a hard upper limit of 2MB for the serialised size of an SDV

The main changes to `VersionName` required to produce the proposed `Version` class are:

- it will contain the actual data rather than an ID of an `ImmutableData` chunk which contains the data
- it will contain a non-optional signature of the contents of that version


## Details

#### Uses of an SDV

1. a user's session data including their main keychain containing all their public and private keys
2. a user's publicly-searchable personal info (all optional), e.g. real name, DOB, address, phone number
3. a private (encrypted) directory listing
4. a public (unencrypted) directory listing
5. a safecoin

**Note**: We'd also considered using SDVs as MPID message outboxes, (an outbox being a collection of messages for other MPIDs which can be retrieved and removed by the other MPID owners).  However, there are two main differences between the current notion of an SDV and what is required for an outbox. Firstly, the outbox messages don't naturally fit a tree-type structure.  A single conversation would fit, with the notion of a parent message and one or more child replies, but the outbox would contain just one side of such a conversation, and would contain several conversations.  Secondly, the SDV doesn't have the ability to delete just a single version, only branches.  Hence to use an SDV for the outbox, we'd need to force every message to be a new branch.


#### Identity of an SDV

The ID or name of an SDV will depend on which of the uses above is being considered.  Below is a table of how this fits with the two new additions to the SDV class; the `OwnerID` and `OtherIDs` fields.

| Use Case             | SDV Name                              | `OwnerID` Field | `OtherIDs` Field                        |
| -------------------- | ------------------------------------- | --------------- | --------------------------------------- |
| 1. session           | SHA512 of combo of `Username` & `PIN` | unused          | unused                                  |
| 2. public info       | MPID name                             | unused          | unused                                  |
| 3. private directory | SHA512 of `OwnerID` + `OtherIDs`      | MAID/MSID name  | single `DirectoryID`                    |
| 4. public directory  | SHA512 of `OwnerID` + `OtherIDs`      | MPID name       | single `DirectoryID`                    |
| 5. safecoin          | SHA512 of `OtherIDs`                  | unused          | multiple PMID names from mining attempt |


#### Contents of a `Version`

Currently a `VersionName` contains an index and ID (`uint64_t` and `ImmutableData::Name` respectively).  The index will be unchanged in the replacement `Version` class, but the ID will be replaced with "Serialised Data" and a signature.  The current tree structure should persist, and to this end it's probably worthwhile to keep a hash (maybe not a full SHA512, just a SHA1 or similar) of the data in the `Version` too.  This would mean that the set of `Versions` would be managed in an identical manner to currently.  It would also reduce the burden on the clients and network when adding a new version, since this action requires the parent `Version` to passed in order to build the hierarchy.  Using a hash of the data, the parent version could be passed simply as the index and hash rather than the full <index, data, signature> tuple.


#### SDV Configurations

The table below shows the varying SDV configurations based on use case of SDV:

| Use Case             | `Version::Data`                   | `Version::Data` encrypted | `Version::Data` signed by | Max. Versions | Max. Branches |
| -------------------- | --------------------------------- | ------------------------- | ------------------------- | ------------- | ------------- |
| 1. session           | session data                      | yes                       | MAID                      | 5             | 1             |
| 2. public info       | publicly-searchable personal info | no                        | MPID                      | 2             | 1             |
| 3. private directory | directory contents                | yes                       | MAID or MSID              | unlimited     | unlimited     |
| 4. public directory  | directory contents                | no                        | MPID                      | unlimited     | unlimited     |
| 5. safecoin          | T.B.C.                            | no                        | current owner MPID        | 1             | 1             |


#### New Behaviour

There are a couple of changes also required to the current SDV behaviour:

- If adding a new `Version` causes the size of the SDV to exceed 2MB, old `Version`s will be popped until the new one can be acommodated.  The `Put` function currently returns `boost::optional<VersionName>` - this should be changed to `std::vector<Version>` and the `vector` should be populated by repeated applying the existing pop rules for `Put`.
- When deleting a branch, a new `Version` should be applied to the tip of the remaining branch with the highest index.  This lets a comparison between two snapshots of an SDV which has had a branch deleted have a clearly defined "most recent" copy.
