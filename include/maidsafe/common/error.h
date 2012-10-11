/* Copyright (c) 2012 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAIDSAFE_COMMON_ERROR_H_
#define MAIDSAFE_COMMON_ERROR_H_

#include <string>
#include <system_error>

#include "maidsafe/common/config.h"


namespace maidsafe {

enum class CommonErrors {
  success = 0,
  pending_result,
  unknown,
  null_pointer,
  invalid_node_id,
  invalid_key_size,
  invalid_string_size,
  invalid_parameter,
  invalid_conversion,
  file_too_large,
  uninitialised,
  hashing_error,
  symmetric_encryption_error,
  symmetric_decryption_error,
  compression_error,
  uncompression_error,
  cant_invoke_from_this_thread
};

enum class AsymmErrors {
  keys_generation_error = 1,
  keys_serialisation_error,
  keys_parse_error,
  invalid_private_key,
  invalid_public_key,
  data_empty,
  invalid_file,
  invalid_signature,
  signature_empty,
  encryption_error,
  decryption_error,
  signing_error
};

enum class FobErrors {
  fob_serialisation_error = 1,
  fob_parsing_error
};

enum class LifeStuffErrors {
  // Authentication
  kAuthenticationError = 1,
  kPasswordFailure,
  kUserDoesntExist,
  kUserExists,
  kPublicUsernameExists,
  kPublicUsernameAlreadySet,
  kFailedToDeleteOldPacket,
  kBadPacket,
  // BufferPacketHandler
  kBPError,
  kBPSerialiseError,
  kBPInfoSerialiseError,
  kBPParseError,
  kBPInfoParseError,
  kStoreNewBPError,
  kModifyBPError,
  kBPAddUserError,
  kBPStoreAddedUserError,
  kBPDeleteUserError,
  kBPStoreDeletedUserError,
  kBPRetrievalError,
  kBPMessagesRetrievalError,
  kGetBPInfoError,
  kBPAddMessageError,
  kBPAwaitingCallback,
  kBPGetPresenceError,
  kBPAddPresenceError,
  // Chunkstore
  kInvalidChunkType,
  kChunkstoreError,
  kChunkFileDoesntExist,
  kErrorReadingChunkFile,
  kChunkstoreUninitialised,
  kChunkstoreFailedStore,
  kChunkstoreFailedDelete,
  kChunkstoreException,
  kHashCheckFailure,
  kChunkExistsInChunkstore,
  // ClientController
  kClientControllerError,
  kClientControllerNotInitialised,
  // DataAtlasHandler
  kDataAtlasError,
  kDBDoesntExist,
  kDBOpenException,
  kDBCreateException,
  kDBReadWriteException,
  kDBCloseException,
  kDBCantFindFile,
  kDBCantFindDirKey,
  kParseDataMapError,
  kAddElementError,
  kModifyElementError,
  kRemoveElementError,
  kRenameElementError,
  kCopyElementError,
  kDataAtlasException,
  // StoreManagers
  kStoreManagerError,
  kStoreManagerInitError,
  kNotConnected,
  kLoadChunkFindNodesFailure,
  kStoreChunkFindNodesFailure,
  kStoreChunkError,
  kChunkNotInChunkstore,
  kGetRequestSigError,
  kGetStorePeerError,
  kSendPrepResponseUninitialised,
  kSendPrepPeerError,
  kSendPrepSignedSizeAltered,
  kSendPrepFailure,
  kSendPrepInvalidId,
  kSendPrepInvalidResponseSignature,
  kSendPrepInvalidContractSignature,
  kSendContentFailure,
  kSendChunkFailure,
  kTaskCancelledOffline,
  kFindNodesError,
  kFindNodesFailure,
  kFindNodesParseError,
  kFindValueError,
  kFindValueFailure,
  kFindValueParseError,
  kLoadChunkFailure,
  kDeleteChunkFindNodesFailure,
  kDeleteChunkError,
  kDeleteSizeError,
  kDeleteChunkFailure,
  kLoadedChunkEmpty,
  kGetChunkFailure,
  kSendPacketError,
  kSendPacketFailure,
  kSendPacketFindValueFailure,
  kSendPacketCached,
  kSendPacketAlreadyExists,
  kSendPacketUnknownExistsType,
  kSendPacketParseError,
  kDeletePacketFindValueFailure,
  kDeletePacketError,
  kDeletePacketParseError,
  kDeletePacketFailure,
  kLoadPacketCached,
  kLoadPacketFailure,
  kPacketUnknownType,
  kDirUnknownType,
  kStoreManagerException,
  kFindAccountHoldersError,
  kRequestPendingConsensus,
  kRequestFailedConsensus,
  kRequestInsufficientResponses,
  kNoPublicKeyToCheck,
  kInvalidPublicKey,
  kKeyUnique,
  kKeyNotUnique,
  kUpdatePacketFailure,
  kUpdatePacketError,
  kUpdatePacketParseError,
  kChunkStorePending,
  kAmendAccountFailure,
  kModifyChunkFailure,
  // KadOps
  kKadConfigException,
  kKadOpsInitFailure,
  kKadIdError,
  // MessageHandler
  kConnectionNotExists,
  kFailedToConnect,
  kFailedToSend,
  kFailedToStartHandler,
  kHandlerAlreadyStarted,
  kHandlerNotStarted,
  kConnectionAlreadyExists,
  kConnectionDown,
  // Session & FileSystem
  kEmptyConversationId,
  kNonExistentConversation,
  kExistingConversation,
  kLoadKeysFailure,
  kGetKeyFailure,
  kContactListFailure,
  kSessionNameEmpty,
  kFileSystemMountError,
  kFileSystemUnmountError,
  kFuseMountPointError,
  kFileSystemException,
  kAddLiveContactFailure,
  kLiveContactNotFound,
  kLiveContactNoEp,
  // SelfEncryptionHandler(-12000)
  kGeneralEncryptionError,
  kEncryptFileFailure,
  kEncryptStringFailure,
  kEncryptDbFailure,
  kDecryptFileFailure,
  kDecryptStringFailure,
  kDecryptDbFailure,
  kEncryptionLocked,
  kEncryptionLink,
  kEncryptionChunk,
  kEncryptionNotForProcessing,
  kEncryptionUnknownType,
  kEncryptionMDMFailure,
  kEncryptionDAHFailure,
  kEncryptionDMFailure,
  kEncryptionSMFailure,
  kEncryptionSmallInput,
  kEncryptionKeyGenFailure,
  kEncryptionGetDirKeyFailure,
  kEncryptionDbMissing,
  kEncryptionDbException,
  kEncryptionDmNotInMap,
  // StoreManagerTaskHandler
  kStoreManagerTaskHandlerError,
  kStoreManagerTaskIncorrectParameter,
  kStoreManagerTaskIncorrectOperation,
  kStoreManagerTaskParentNotActive,
  kStoreManagerTaskNotFound,
  kStoreManagerTaskCancelledOrDone,
  kStoreManagerTaskConflict,
  // Validator
  kValidatorNoParameters,
  kValidatorNoPrivateKey,
  kInvalidPointer,
  kTimedOut,
  // DataStore
  kEmptyKey,
  kZeroTTL,
  kFailedToModifyKeyValue,
  // RoutingTable
  kOwnIdNotIncludable,
  kFailedToInsertNewContact,
  kFailedToFindContact,
  kFailedToSetPublicKey,
  kFailedToUpdateRankInfo,
  kFailedToSetPreferredEndpoint,
  kFailedToIncrementFailedRpcCount,
  // Node
  kNoOnlineBootstrapContacts,
  kInvalidBootstrapContacts,
  kNotListening,
  kNotJoined,
  kResponseTimeout,
  kResponseCancelled,
  kAnonymousSessionEnded,
  kInvalidDestinationId,
  kEmptyData,
  kTypeNotAllowed,
  kFailedtoSendFindNode,
  kDataSizeNotAllowed,
  kFailedtoGetEndpoint,
  kPartialJoinSessionEnded,
  // DirectoryListing
  kFailedToAddChild,
  kFailedToRemoveChild,
  // DirectoryListingHandler
  kFailedToInitialise,
  kFailedToGetDirectoryData,
  kFailedToAddDirectoryListing,
  kFailedToDeleteDirectoryListing,
  kFailedToRenameDirectoryListing,
  kFailedToCreateDirectory,
  kFailedToSaveParentDirectoryListing,
  kFailedToSaveChanges,
  kFailedToDeleteDirectoryListingNotEmpty,
  kFailedToStoreEncryptedDataMap,
  kFailedToModifyEncryptedDataMap,
  kFailedToDeleteEncryptedDataMap,
  kFailedToDecryptDataMap,
  kFailedToParseShares,
  kNotAuthorised,
  kNestedShareDisallowed,
  kHiddenNotAllowed,
  kFailedToRetrieveData,
  kInvalidDataMap,
  kFailedToGetLock,
  // DriveInUserSpace
  kChildAlreadyExists,
  kFailedToGetChild,
  kFailedChunkStoreInit,
  kCBFSError,
  kCreateStorageError,
  kMountError,
  kFuseFailedToParseCommandLine,
  kFuseFailedToMount,
  kFuseNewFailed,
  kFuseFailedToDaemonise,
  kFuseFailedToSetSignalHandlers,
  kUnmountError,
  kInvalidSelfEncryptor,
  kReadError,
  kWriteError,
  kInvalidSeek,
  kInvalidPath,
  kFailedToGetMetaData,
  kNoDataMap,
  kFailedToSerialiseDataMap,
  kFailedToParseDataMap,
  kNoDirectoryId,
  kInvalidIds,
  kInvalidKey,
  kParentShared,
  kFailedToUpdateShareKeys,
  kFailedToGetShareKeys,
  kNoMsHidden,
  kMsHiddenAlreadyExists,
  kShareAlreadyExistsInHierarchy,
  kDirectoryRecursionException,
  // meta_data_ops
  kSerialisingError,
  kParsingError,
  // Shares
  kFailedToParseShareUsers,
  kFailedToSerialiseShareUsers,
  kShareUserAlreadyExists,
  kFailedToFindShareUser,
  kShareByIdNotFound,
  kNotBootstrapped,
  kFull,
  kInvalidTransport,
  kInvalidConnection,
  kNotConnectable,
  kInvalidEndpoint,
  kTransportStartFailure,
  kEmptyValidationData,
  kConnectError,
  kMessageTooLarge,
  kPingFailed,
  kWontPingAlreadyConnected,
  kWontPingOurself,
  kConnectAttemptAlreadyRunning,
  kOwnId,
  kNoPendingConnectAttempt,
  kBootstrapUpgradeFailure,
  kInvalidParameter,
  kNoBootstrapEndpoints,
  kFailedToGetLocalAddress,

  // Upper limit of values for this enum.
  kReturnCodeLimit,
  kGeneralError,
  kUnknownFailure,
  kParseFailure,
  kPreOperationCheckFailure,
  kDuplicateNameFailure,
  kVerifyDataFailure,
  kGetFailure,
  kStoreFailure,
  kDeleteFailure,
  kModifyFailure,
  kInvalidSignedData,
  kFailedSignatureCheck,
  kNotHashable,
  kNotOwner,
  kFailedToFindChunk,
  kAppendDisallowed,
  kHashFailure,
  kDifferentVersion,
  kChunkNotModified,
  kDataNotPublicKey,
  // DownloadManager
  kManifestFailure,
  kDownloadFailure,
  kNoVersionChange,
  kLocalFailure,
  // Transport
  kListenError,
  kMessageSizeTooLarge,
  kReceiveFailure,
  kReceiveTimeout,
  kSendTimeout,
  kConnectFailure,
  kReadOnlyRestrictedSuccess,
  kRemoteChunkStoreFailure,
  kPublicIdNotFoundFailure,
  kGetPublicIdError,
  // LifeStuffImpl and API
  kWrongState,
  kWrongLoggedInState,
  kWrongAccessLevel,
  kInitialiseUpdateFunctionFailure,
  kInitialiseBootstrapsFailure,
  kInitialiseChunkStoreFailure,
  kSetSlotsFailure,
  kConnectSignalsFailure,
  kLogoutCredentialsFailure,
  kLogoutCompleteChunkFailure,
  kCreateDirectoryError,
  kMountDriveOnCreationError,
  kCreateMyStuffError,
  kCreateSharedStuffError,
  kMountDriveTryManualUnMount,
  kMountDriveMountPointCreationFailure,
  kMountDriveError,
  kUnMountDriveError,
  kStartMessagesAndContactsNoPublicIds,
  kChangePictureWrongSize,
  kChangePictureWriteHiddenFileFailure,
  kChangePictureEmptyDataMap,
  kChangePictureReconstructionError,
  kSendMessageSizeFailure,
  kAcceptFilePathError,
  kAcceptFileSerialisedIdentifierEmpty,
  kAcceptFileGetFileNameDataFailure,
  kAcceptFileCorruptDatamap,
  kAcceptFileVerifyCreatePathFailure,
  kAcceptFileNameFailure,
  kReadHiddenFileContentFailure,
  kCheckPasswordFailure,
  kVaultCreationCredentialsFailure,
  kVaultCreationStartFailure,
  kNoShareTarget,
  kCouldNotAcquirePmidKeys,
  // Account Locking
  kLidParseToSignedDataFailure,
  kLidDecryptDataFailure,
  kLidParseToLockingPacketFailure,
  kLidAddItemIdentifierInUse,
  kLidAddItemFullAccessUnavailable,
  kLidRemoveItemIdentifierNotFound,
  kLidRemoveItemsIdentifierNotFound,
  kLidUpdateTimestampIdentifierNotFound,
  kLidCheckOthersIdentifierNotFound,
  kAccountAlreadyLoggedIn,
  kLidNotFound,
  kLidIdentifierFound,
  // Contacts
  kContactInsertionFailure,
  kContactErasureFailure,
  kContactNotPresentFailure,
  kContactReplacementFailure,
  // MessageHander
  kStartMessagesNoPublicIds,
  kPublicIdTimeout,
  kMessageHandlerException,
  kCannotConvertInboxItemToProtobuf,
  kContactInfoContentsFailure,
  // PublicID
  kStartContactsNoPublicIds,
  kGetPublicKeyFailure,
  kContactNotFoundFailure,
  kSigningError,
  kEncryptingError,
  kPublicIdException,
  kSendContactInfoFailure,
  kStorePublicIdFailure,
  kModifyAppendabilityFailure,
  kGenerateNewMMIDFailure,
  kRemoveContactFailure,
  kDeletePublicIdFailure,
  kCannotAddOwnPublicId,
  kCanOnlyRejectPendingResponseContact,
  kConfirmContactGetInfoFailure,
  kConfirmContactInformFailure,
  kConfirmContactStatusFailure,
  kPRWERGetInfoFailure,
  kPRWERPublicKeyFailure,
  kPRWERInformFailure,
  kPRWERStatusFailure,
  // Session
  kTryAgainLater,
  kPublicIdInsertionFailure,
  kParseDataAtlasTmidEmpty,
  kParseDataAtlasTmidDoesNotParse,
  kParseDataAtlasKeyringDoesNotParse,
  kSerialiseDataAtlasKeyringFailure,
  kSerialiseDataAtlasToStringFailure,
  // UserCredentials
  kChangePasswordFailure,
  kLoginUserNonExistence,
  kLoginAccountCorrupted,
  kLoginSessionNotYetSaved,
  kLoginUsingNextToLastSession,
  kMustDieFailure,
  kCorruptedPacket,
  kIdPacketNotFound,
  kTemporaryIdPacketNotFound,
  kSetIdentityPacketsFailure,
  kStoreIdentityPacketsFailure,
  kDeleteIdentityPacketsFailure,
  kCreateSignaturePacketInfoFailure,
  kCreateSignaturePacketsFailure,
  kDeleteSignaturePacketsFailure,
  kSessionFailure,
  kSessionSerialisationFailure,
  kSaveSessionFailure,
  kUsingNextToLastSession,
  // UserStorage
  kOwnerTryingToLeave,
  // Utils
  kWordSizeInvalid,
  kWordPatternInvalid,
  kKeywordSizeInvalid,
  kKeywordPatternInvalid,
  kPinSizeInvalid,
  kPinPatternInvalid,
  kPasswordSizeInvalid,
  kPasswordPatternInvalid,
  kPublicIdEmpty,
  kPublicIdLengthInvalid,
  kPublicIdEndSpaceInvalid,
  kPublicIdDoubleSpaceInvalid,
  kAtLeastOneFailure,
  // Codes remaining in DISABLED tests.  Expect these codes to be redundant soon.
  kReadOnlyFailure,
  kFailedSymmDecrypt
};

std::error_code make_error_code(CommonErrors code);
std::error_code make_error_code(AsymmErrors code);
std::error_code make_error_code(FobErrors code);
std::error_code make_error_code(LifeStuffErrors code);
std::error_condition make_error_condition(CommonErrors code);
std::error_condition make_error_condition(AsymmErrors code);
std::error_condition make_error_condition(FobErrors code);
std::error_condition make_error_condition(LifeStuffErrors code);

void DoThrowError(const std::error_code& code);

inline void ThrowError(const std::error_code& code) {
  if (code)
    DoThrowError(code);
}

const std::error_category& GetCommonCategory();
const std::error_category& GetAsymmCategory();
const std::error_category& GetFobCategory();
const std::error_category& GetLifeStuffCategory();

}  // namespace maidsafe


namespace std {

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Weffc++"
#endif

template <>
struct is_error_code_enum<maidsafe::CommonErrors> : public true_type {};  // NOLINT (dirvine)

template <>
struct is_error_code_enum<maidsafe::AsymmErrors> : public true_type {};  // NOLINT (dirvine)

template <>
struct is_error_code_enum<maidsafe::FobErrors> : public true_type {};  // NOLINT (dirvine)

template <>
struct is_error_code_enum<maidsafe::LifeStuffErrors> : public true_type {};  // NOLINT (dirvine)


#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

}  // namespace std

#endif  // MAIDSAFE_COMMON_ERROR_H_
