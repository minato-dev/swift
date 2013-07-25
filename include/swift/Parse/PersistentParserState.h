//===--- PersistentParserState.h - Parser State -----------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// Parser state persistent across multiple parses.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_PARSE_PERSISTENTPARSERSTATE_H
#define SWIFT_PARSE_PERSISTENTPARSERSTATE_H

#include "swift/Basic/SourceLoc.h"
#include "swift/Parse/Scope.h"
#include "llvm/ADT/DenseMap.h"

namespace swift {
  class FuncExpr;

/// \brief Parser state persistent across multiple parses.
class PersistentParserState {
public:
  struct ParserPos {
    SourceLoc Loc;
    SourceLoc PrevLoc;

    bool isValid() const { return Loc.isValid(); }
  };

  class FunctionBodyState {
    ParserPos BodyPos;
    SavedScope Scope;
    friend class Parser;

    SavedScope takeScope() {
      return std::move(Scope);
    }

  public:
    FunctionBodyState(SourceRange BodyRange, SourceLoc PreviousLoc,
                      SavedScope &&Scope)
      : BodyPos{BodyRange.Start, PreviousLoc}, Scope(std::move(Scope))
    {}
  };

private:
  ScopeInfo ScopeInfo;
  typedef llvm::DenseMap<FuncExpr *, std::unique_ptr<FunctionBodyState>>
      DelayedBodiesTy;
  DelayedBodiesTy DelayedBodies;
  /// \brief Parser sets this if it stopped parsing before the buffer ended.
  ParserPos MarkedPos;

public:
  swift::ScopeInfo &getScopeInfo() { return ScopeInfo; }

  void delayFunctionBodyParsing(FuncExpr *FE, SourceRange BodyRange,
                                SourceLoc PreviousLoc);
  std::unique_ptr<FunctionBodyState> takeBodyState(FuncExpr *FE);

  void markParserPosition(SourceLoc Loc, SourceLoc PrevLoc) {
    MarkedPos = {Loc, PrevLoc};
  }

  /// \brief Returns the marked parser position and resets it.
  ParserPos takeParserPosition() {
    ParserPos Pos = MarkedPos;
    MarkedPos = ParserPos();
    return Pos;
  }
};

} // end namespace swift

#endif
