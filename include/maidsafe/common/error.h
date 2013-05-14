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

#include "boost/exception/all.hpp"

#include "maidsafe/common/config.h"


namespace maidsafe {

class maidsafe_error : public std::system_error {
 public:
  maidsafe_error(std::error_code ec, const std::string& what_arg)
      : std::system_error(ec, what_arg) {}
  maidsafe_error(std::error_code ec, const char* what_arg) : std::system_error(ec, what_arg) {}
  explicit maidsafe_error(std::error_code ec) : std::system_error(ec) {}
  maidsafe_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : std::system_error(ev, ecat, what_arg) {}
  maidsafe_error(int ev, const std::error_category& ecat, const char* what_arg)
      : std::system_error(ev, ecat, what_arg) {}
  maidsafe_error(int ev, const std::error_category& ecat) : std::system_error(ev, ecat) {}
};

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
  cannot_invoke_from_this_thread,
  cannot_exceed_limit,
  filesystem_io_error,
  no_such_element,
  serialisation_error,
  parsing_error,
  not_a_directory
};

class common_error : public maidsafe_error {
 public:
  common_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  common_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit common_error(std::error_code ec) : maidsafe_error(ec) {}
  common_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  common_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  common_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(CommonErrors code);
std::error_condition make_error_condition(CommonErrors code);
const std::error_category& GetCommonCategory();
common_error MakeError(CommonErrors code);


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

class asymm_error : public maidsafe_error {
 public:
  asymm_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  asymm_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit asymm_error(std::error_code ec) : maidsafe_error(ec) {}
  asymm_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  asymm_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  asymm_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(AsymmErrors code);
std::error_condition make_error_condition(AsymmErrors code);
const std::error_category& GetAsymmCategory();
asymm_error MakeError(AsymmErrors code);


enum class PassportErrors {
  fob_serialisation_error = 1,
  fob_parsing_error,
  mid_parsing_error,
  tmid_parsing_error,
  no_confirmed_fob,
  no_pending_fob,
  passport_parsing_error,
  public_id_already_exists,
  no_such_public_id
};

class passport_error : public maidsafe_error {
 public:
  passport_error(std::error_code ec, const std::string& what_arg)
      : maidsafe_error(ec, what_arg) {}
  passport_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit passport_error(std::error_code ec) : maidsafe_error(ec) {}
  passport_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  passport_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  passport_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(PassportErrors code);
std::error_condition make_error_condition(PassportErrors code);
const std::error_category& GetPassportCategory();
passport_error MakeError(PassportErrors code);


enum class NfsErrors {
  failed_to_get_data = 1
};

class nfs_error : public maidsafe_error {
 public:
  nfs_error(std::error_code ec, const std::string& what_arg) : maidsafe_error(ec, what_arg) {}
  nfs_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit nfs_error(std::error_code ec) : maidsafe_error(ec) {}
  nfs_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  nfs_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  nfs_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(NfsErrors code);
std::error_condition make_error_condition(NfsErrors code);
const std::error_category& GetNfsCategory();
nfs_error MakeError(NfsErrors code);



enum class RoutingErrors {
  timed_out = 1,
  not_in_range,
  not_connected
};

class routing_error : public maidsafe_error {
 public:
  routing_error(std::error_code ec, const std::string& what_arg)
      : maidsafe_error(ec, what_arg) {}
  routing_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit routing_error(std::error_code ec) : maidsafe_error(ec) {}
  routing_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  routing_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  routing_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(RoutingErrors code);
std::error_condition make_error_condition(RoutingErrors code);
const std::error_category& GetRoutingCategory();
routing_error MakeError(RoutingErrors code);



enum class VaultErrors {
  failed_to_join_network = 1,
  failed_to_handle_request,
  operation_not_supported,
  permission_denied,
  no_such_account,
  low_space,
  not_enough_space,
  unique_data_clash,
  data_available_not_given
};

class vault_error : public maidsafe_error {
 public:
  vault_error(std::error_code ec, const std::string& what_arg)
      : maidsafe_error(ec, what_arg) {}
  vault_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit vault_error(std::error_code ec) : maidsafe_error(ec) {}
  vault_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  vault_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(VaultErrors code);
std::error_condition make_error_condition(VaultErrors code);
const std::error_category& GetVaultCategory();
vault_error MakeError(VaultErrors code);



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

class lifestuff_error : public maidsafe_error {
 public:
  lifestuff_error(std::error_code ec, const std::string& what_arg)
      : maidsafe_error(ec, what_arg) {}
  lifestuff_error(std::error_code ec, const char* what_arg) : maidsafe_error(ec, what_arg) {}
  explicit lifestuff_error(std::error_code ec) : maidsafe_error(ec) {}
  lifestuff_error(int ev, const std::error_category& ecat, const std::string& what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  lifestuff_error(int ev, const std::error_category& ecat, const char* what_arg)
      : maidsafe_error(ev, ecat, what_arg) {}
  lifestuff_error(int ev, const std::error_category& ecat) : maidsafe_error(ev, ecat) {}
};

std::error_code make_error_code(LifeStuffErrors code);
std::error_condition make_error_condition(LifeStuffErrors code);
const std::error_category& GetLifeStuffCategory();
lifestuff_error MakeError(LifeStuffErrors code);



template<typename MaidsafeErrorCode>
inline void ThrowError(const MaidsafeErrorCode& code) {
  auto error(MakeError(code));
  if (error.code())
    boost::throw_exception(error);
}

}  // namespace maidsafe



namespace std {

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Weffc++"
#endif

template <>
struct is_error_code_enum<maidsafe::CommonErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::AsymmErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::PassportErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::NfsErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::RoutingErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::VaultErrors> : public true_type {};

template <>
struct is_error_code_enum<maidsafe::LifeStuffErrors> : public true_type {};


#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

}  // namespace std

#endif  // MAIDSAFE_COMMON_ERROR_H_
